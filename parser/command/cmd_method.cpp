/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "base/macros.h"
#include "command/cmd_method.h"
#include "command/command_manager.h"
#include "base/utils.h"
#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
#include "runtime/oat.h"
#include "runtime/oat/stack_map.h"
#include "runtime/nterp_helpers.h"
#include "common/disassemble/capstone.h"
#include <unistd.h>
#include <getopt.h>
#include <iomanip>
#include <stdlib.h>

bool MethodCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"dex-dump",    no_argument,       0,  0 },
        {"oat-dump",    no_argument,       0,  1 },
        {"pc",          required_argument, 0,  2 },
        {"inst",        required_argument, 0, 'i'},
        {"num",         required_argument, 0, 'n'},
        {"verbose",     no_argument,       0, 'v'},
        {"binary",      no_argument,       0, 'b'},
        {0,               0,               0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "i:n:01bv",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                dump_opt |= METHOD_DUMP_DEXCODE;
                break;
            case 1:
                dump_opt |= METHOD_DUMP_OATCODE;
                break;
        }
    }

    if (dump_opt & METHOD_DUMP_OATCODE) {
        Android::OatPrepare();
    }
    return true;
}

int MethodCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return 0;

    int opt;
    dump_opt = METHOD_DUMP_NAME;
    verbose = false;
    count = 0;
    pc = 0x0;

    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"dex-dump",    no_argument,       0,  0 },
        {"oat-dump",    no_argument,       0,  1 },
        {"pc",          required_argument, 0,  2 },
        {"inst",        required_argument, 0, 'i'},
        {"num",         required_argument, 0, 'n'},
        {"verbose",     no_argument,       0, 'v'},
        {"binary",      no_argument,       0, 'b'},
        {0,               0,               0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "i:n:012bv",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                instref = Utils::atol(optarg);
                break;
            case 'n':
                count = atoi(optarg);
                break;
            case 0:
                dump_opt |= METHOD_DUMP_DEXCODE;
                break;
            case 1:
                dump_opt |= METHOD_DUMP_OATCODE;
                break;
            case 2:
                pc = Utils::atol(optarg);
                break;
            case 'v':
                verbose = true;
                break;
            case 'b':
                dump_opt |= METHOD_DUMP_BINARY;
                break;
        }
    }

    if (optind >= argc) {
        usage();
        return 0;
    }

    method = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
    uint32_t dex_method_idx = method.GetDexMethodIndex();
    if (LIKELY(dex_method_idx != art::dex::kDexNoIndex)) {
        LOGI("%s%s [dex_method_idx=%d]\n", art::PrettyJavaAccessFlags(method.access_flags()).c_str(),
                       method.PrettyMethod().c_str(), dex_method_idx);

        if (dump_opt & METHOD_DUMP_DEXCODE)
            Dexdump();

        if (dump_opt & METHOD_DUMP_OATCODE)
            Oatdump();

        if (dump_opt & METHOD_DUMP_BINARY)
            Binarydump();

    } else {
        LOGI("%s\n", method.PrettyMethod().c_str());
    }
    return 0;
}

void MethodCommand::Dexdump() {
    art::dex::CodeItem item = method.GetCodeItem();
    art::DexFile& dex_file = method.GetDexFile();
    if (item.Ptr()) {
        if (verbose) {
            LOGI("Location      : %s\n", dex_file.GetLocation().c_str());
            LOGI("CodeItem      : 0x%lx\n", item.Ptr());
            LOGI("Registers     : %d\n", item.num_regs_);
            LOGI("Ins           : %d\n", item.ins_size_);
            LOGI("Outs          : %d\n", item.out_regs_);
            LOGI("Insns size    : 0x%x\n", item.insns_count_);
        }

        LOGI("DEX CODE:\n");
        api::MemoryRef coderef = item.Ptr() + item.code_offset_;
        api::MemoryRef endref = coderef.Ptr() + (item.insns_count_ << 1);

        if (instref >= coderef && instref < endref) {
            coderef = instref;
            endref = instref.Ptr() + art::Dexdump::GetDexInstSize(coderef);
        }

        if (count > 0) {
            endref = item.Ptr() + item.code_offset_ + (item.insns_count_ << 1);
        }

        coderef.copyRef(item);

        int current = 0;
        while (coderef < endref) {
            LOGI("  %s\n", art::Dexdump::PrettyDexInst(coderef, dex_file).c_str());
            current++;
            if (count && current == count)
                break;
            coderef.MovePtr(art::Dexdump::GetDexInstSize(coderef));
        }
    } else {
        LOGI("  NO CODE!\n");
    }
}

void MethodCommand::Oatdump() {
    art::OatQuickMethodHeader method_header = method.GetOatQuickMethodHeader(/*method.GetEntryPointFromQuickCompiledCode()*/ pc);
    if (method_header.Ptr()) {
        if (verbose) {
            method_header.Dump("");
            art::QuickMethodFrameInfo frame;
            if (method_header.IsOptimized()) {
                art::CodeInfo code_info = art::CodeInfo::Decode(method_header.GetOptimizedCodeInfoPtr());
                code_info.ExtendNumRegister(method);
                code_info.Dump("    ");
                frame = method_header.GetFrameInfo();
                LOGI("  QuickMethodFrameInfo\n");
            } else if (method_header.IsNterpMethodHeader()) {
                frame = NterpFrameInfo(method);
                LOGI("  NterpFrameInfo\n");
            }
            LOGI("    frame_size_in_bytes: 0x%x\n", frame.FrameSizeInBytes());
            LOGI("    core_spill_mask: 0x%x %s\n", frame.CoreSpillMask(), art::QuickMethodFrameInfo::PrettySpillMask(frame.CoreSpillMask()).c_str());
            LOGI("    fp_spill_mask: 0x%x %s\n", frame.FpSpillMask(), art::QuickMethodFrameInfo::PrettySpillMask(frame.FpSpillMask()).c_str());
        }
        LOGI("OAT CODE:\n");
        LOGI("  [0x%lx, 0x%lx]\n", method_header.GetCodeStart(), method_header.GetCodeStart() + method_header.GetCodeSize());
        capstone::Disassember::Dump("  ", method_header.GetCodeStart(), method_header.GetCodeSize());
    }
}

void MethodCommand::Binarydump() {
    LOGI("Binary:\n");
    int argc = 4;
    std::stringstream begin;
    std::stringstream end;
    begin << std::hex << method.Ptr();
    end << std::hex << (method.Ptr() + SIZEOF(ArtMethod));
    std::string bs = begin.str();
    std::string es = end.str();
    char* argv[4] = {
        const_cast<char*>("rd"),
        const_cast<char*>(bs.c_str()),
        const_cast<char*>("-e"),
        const_cast<char*>(es.c_str())};
    CommandManager::Execute(argv[0], argc, argv);
}

void MethodCommand::usage() {
    LOGI("Usage: method <ArtMethod> [option..]\n");
    LOGI("Option:\n");
    LOGI("    --dex-dump: show dalvik byte codes\n");
    LOGI("    --oat-dump: show oat machine codes\n");
    LOGI("    --binary|-b: show ArtMethod memory\n");
    LOGI("    --inst|-i: show instpc byte code\n");
    LOGI("    --num|-n: maxline num\n");
    LOGI("    --verbaose|-v: more info\n");
}
