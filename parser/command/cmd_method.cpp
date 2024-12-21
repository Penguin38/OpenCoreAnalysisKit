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
#include "command/backtrace/cmd_frame.h"
#include "command/command_manager.h"
#include "base/utils.h"
#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
#include "runtime/oat.h"
#include "runtime/oat/stack_map.h"
#include "runtime/nterp_helpers.h"
#include "common/disassemble/capstone.h"
#include "common/elf.h"
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
                count = std::atoi(optarg);
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
        LOGI(ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_LIGHTRED "%s" ANSI_COLOR_RESET " [dex_method_idx=%d]\n", art::PrettyJavaAccessFlags(method.access_flags()).c_str(),
                       method.ColorPrettyMethod().c_str(), dex_method_idx);

        if (dump_opt & METHOD_DUMP_DEXCODE)
            Dexdump();

        if (dump_opt & METHOD_DUMP_OATCODE)
            Oatdump();

        if (dump_opt & METHOD_DUMP_BINARY)
            Binarydump();

    } else {
        LOGI("%s\n", method.ColorPrettyMethod().c_str());
    }
    return 0;
}

void MethodCommand::Dexdump() {
    art::dex::CodeItem item = method.GetCodeItem();
    art::DexFile& dex_file = method.GetDexFile();
    if (item.Ptr()) {
        if (verbose) {
            LOGI("Location      : %s\n", dex_file.GetLocation().c_str());
            LOGI("CodeItem      : 0x%" PRIx64 "\n", item.Ptr());
            LOGI("Registers     : %d\n", item.num_regs_);
            LOGI("Ins           : %d\n", item.ins_size_);
            LOGI("Outs          : %d\n", item.out_regs_);
            LOGI("Insns size    : 0x%x\n", item.insns_count_);
        }

        LOGI(ANSI_COLOR_LIGHTRED "DEX CODE:\n" ANSI_COLOR_RESET);
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
        LOGI(ANSI_COLOR_RED "  NO CODE!\n" ANSI_COLOR_RESET);
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
        LOGI(ANSI_COLOR_RED "OAT CODE:\n" ANSI_COLOR_RESET);
        LOGI("  [0x%" PRIx64 ", 0x%" PRIx64 "]\n", method_header.GetCodeStart(), method_header.GetCodeStart() + method_header.GetCodeSize());
        if (method_header.IsOptimized()) {
            capstone::Disassember::Option opt(method_header.GetCodeStart(), -1);
            if (CoreApi::GetMachine() == EM_ARM) {
                opt.SetArchMode(capstone::Disassember::Option::ARCH_ARM, capstone::Disassember::Option::MODE_THUMB);
            }
            uint64_t subid = 0;
            std::vector<art::GeneralStackMap> submaps;
            method_header.NativeStackMaps(submaps);
            art::dex::CodeItem item = method.GetCodeItem();
            uint64_t start = method_header.GetCodeStart();
            uint64_t code_size = 0;
            for (const auto& stack : submaps) {
                capstone::Disassember::Dump("  ", start, stack.native_pc - code_size, opt);
                LOGI("    GeneralStackMap[%" PRIu64 "] (NativePc=0x%" PRIx64 " DexPc=0x%" PRIx64 ")\n",
                        subid++, stack.native_pc + method_header.GetCodeStart(),
                        item.Ptr() ? item.Ptr() + item.code_offset_ + 0x2 * stack.dex_pc : 0x0);
                std::map<uint32_t, art::DexRegisterInfo> vregs;
                method_header.NativePc2VRegs(start - method_header.GetCodeStart(), vregs);
                FrameCommand::ShowJavaFrameRegister("      ", vregs);
                start = stack.native_pc + method_header.GetCodeStart();
                code_size = stack.native_pc;
            }
            capstone::Disassember::Dump("  ", start, method_header.GetCodeSize() - code_size, opt);
        }
    }
}

void MethodCommand::Binarydump() {
    LOGI("Binary:\n");
    int argc = 4;
    std::string bs = Utils::ToHex(method.Ptr());
    std::string es = Utils::ToHex(method.Ptr() + SIZEOF(ArtMethod));
    char* argv[4] = {
        const_cast<char*>("rd"),
        const_cast<char*>(bs.c_str()),
        const_cast<char*>("-e"),
        const_cast<char*>(es.c_str())};
    CommandManager::Execute(argv[0], argc, argv);
}

void MethodCommand::usage() {
    LOGI("Usage: method <ART_METHOD> [OPTIONE...]\n");
    LOGI("Option:\n");
    LOGI("    --dex-dump        show dalvik byte codes\n");
    LOGI("    -i, --inst <PC>   only dex-dump, show instpc byte code\n");
    LOGI("    -n, --num <NUM>   only dex-dump, show maxline num\n");
    LOGI("    --oat-dump        show oat machine codes\n");
    LOGI("        --pc <PC>     only oat-dump\n");
    LOGI("    -b, --binary      show ArtMethod memory\n");
    LOGI("    -v, --verbaose    show more info\n");
    ENTER();
    LOGI("core-parser> method 0x70b509c0 -v --dex-dump --oat-dump\n");
    LOGI("public static void com.android.internal.os.ZygoteInit.main(java.lang.String[]) [dex_method_idx=49967]\n");
    LOGI("Location      : /system/framework/framework.jar!classes3.dex\n");
    LOGI("CodeItem      : 0x79185a704704\n");
    LOGI("Registers     : 19\n");
    LOGI("Ins           : 1\n");
    LOGI("Outs          : 4\n");
    LOGI("Insns size    : 0x167\n");
    LOGI("DEX CODE:\n");
    LOGI("  0x79185a704714: 0108 0012                | move-object/from16 v1, v18\n");
    LOGI("  0x79185a704718: 001a 07bc                | const-string v0, \"--socket-name=\" // string@1980\n");
    LOGI("  0x79185a70471c: 021a 0791                | const-string v2, \"--abi-list=\" // string@1937\n");
    LOGI("  0x79185a704720: 031a 8897                | const-string v3, \"Zygote\" // string@34967\n");
    LOGI("  0x79185a704724: 0412                     | const/4 v4, #+0\n");
    LOGI("  0x79185a704726: 0071 e9ef 0000           | invoke-static {}, void dalvik.system.ZygoteHooks.startZygoteNoThreadCreation() // method@59887\n");
    LOGI("  ...\n");
    LOGI("OatQuickMethodHeader(0x719075d8)\n");
    LOGI("  code_offset: 0x719075e0\n");
    LOGI("  code_size: 0x944\n");
    LOGI("  vmap_table_offset: 0x74d2f2\n");
    LOGI("    CodeInfo BitSize=7614 FrameSize:0xb0 CoreSpillMask:0x1f028 FpSpillMask:0x0 NumberOfDexRegisters:19\n");
    LOGI("      StackMap BitSize=3600 Rows=75 Bits={Kind=1 PackedNativePc=0xc DexPc=0x9 RegisterMaskIndex=4 StackMaskIndex=5 InlineInfoIndex=4 DexRegisterMaskIndex=5 DexRegisterMapIndex=8}\n");
    LOGI("      RegisterMask BitSize=247 Rows=13 Bits={Value=15 Shift=4}\n");
    LOGI("      StackMask BitSize=893 Rows=19 Bits={Mask=47}\n");
    LOGI("      InlineInfo BitSize=130 Rows=10 Bits={IsLast=1 DexPc=4 MethodInfoIndex=3 ArtMethodHi=0 ArtMethodLo=0 NumberOfDexRegisters=5}\n");
    LOGI("      MethodInfo BitSize=112 Rows=7 Bits={MethodIndex=16}\n");
    LOGI("      DexRegisterMask BitSize=744 Rows=31 Bits={Mask=24}\n");
    LOGI("      DexRegisterMap BitSize=906 Rows=151 Bits={CatalogueIndex=6}\n");
    LOGI("      DexRegisterInfo BitSize=702 Rows=39 Bits={Kind=3 PackedValue=15}\n");
    LOGI("  QuickMethodFrameInfo\n");
    LOGI("    frame_size_in_bytes: 0xb0\n");
    LOGI("    core_spill_mask: 0x1f028 (rbx, rbp, r12, r13, r14, r15, rip)\n");
    LOGI("    fp_spill_mask: 0x0 \n");
    LOGI("OAT CODE:\n");
    LOGI("  [0x719075e0, 0x71907f24]\n");
    LOGI("  0x719075e0:         ffffe00024848548 | test qword ptr [rsp - 0x2000], rax\n");
    LOGI("    GeneralStackMap[0] (NativePc=0x719075e8 DexPc=0x79185a704714)\n");
    LOGI("  0x719075e8:                     5741 | push r15\n");
    LOGI("  0x719075ea:                     5641 | push r14\n");
    LOGI("  0x719075ec:                     5541 | push r13\n");
    LOGI("  0x719075ee:                     5441 | push r12\n");
    LOGI("  0x719075f0:                       55 | push rbp\n");
    LOGI("  0x719075f1:                       53 | push rbx\n");
    LOGI("  0x719075f2:                 78ec8348 | sub rsp, 0x78\n");
    LOGI("  0x719075f6:                 243c8948 | mov qword ptr [rsp], rdi\n");
    LOGI("  0x719075fa:           000000b824b489 | mov dword ptr [rsp + 0xb8], esi\n");
    LOGI("  0x71907601:     0000000000253c836665 | cmp word ptr gs:[0], 0\n");
    LOGI("  0x7190760b:             00000893850f | jne 0x71907ea4\n");
    LOGI("  0x71907611:                   f38948 | mov rbx, rsi\n");
    LOGI("  0x71907614:             fecf453e2d8d | lea ebp, [rip - 0x130bac2]\n");
    LOGI("  0x7190761a:                 34246c89 | mov dword ptr [rsp + 0x34], ebp\n");
    LOGI("  ...\n");
}
