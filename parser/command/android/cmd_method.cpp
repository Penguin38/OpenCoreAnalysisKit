/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "base/macros.h"
#include "command/android/cmd_method.h"
#include "command/core/backtrace/cmd_frame.h"
#include "command/command_manager.h"
#include "base/utils.h"
#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
#include "runtime/oat.h"
#include "runtime/oat/stack_map.h"
#include "runtime/nterp_helpers.h"
#include "runtime/interpreter/quick_frame.h"
#include "common/disassemble/capstone.h"
#include "common/elf.h"
#include <unistd.h>
#include <getopt.h>
#include <iomanip>
#include <stdlib.h>

int MethodCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return Command::FINISH;

    options.instpc = 0x0;
    options.dump_opt = METHOD_DUMP_NAME;
    options.verbose = false;
    options.count = 0;
    options.pc = 0x0;
    options.dexpc = 0x0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"dex-dump",    no_argument,       0,  1 },
        {"oat-dump",    no_argument,       0,  2 },
        {"pc",          required_argument, 0,  3 },
        {"inst",        required_argument, 0, 'i'},
        {"num",         required_argument, 0, 'n'},
        {"verbose",     no_argument,       0, 'v'},
        {"binary",      no_argument,       0, 'b'},
        {"search",      required_argument, 0, 's'},
        {0,             0,                 0,  0 },
    };

    while ((opt = getopt_long(argc, argv, "i:n:012bvs:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                options.instpc = Utils::atol(optarg);
                break;
            case 'n':
                options.count = std::atoi(optarg);
                break;
            case 1:
                options.dump_opt |= METHOD_DUMP_DEXCODE;
                break;
            case 2:
                options.dump_opt |= METHOD_DUMP_OATCODE;
                break;
            case 3:
                options.pc = Utils::atol(optarg);
                break;
            case 'v':
                options.verbose = true;
                break;
            case 'b':
                options.dump_opt |= METHOD_DUMP_BINARY;
                break;
            case 's':
                options.dexpc = Utils::atol(optarg);
                break;
        }
    }
    options.optind = optind;

    if (options.optind >= argc && !options.dexpc) {
        usage();
        return Command::FINISH;
    }

    if (options.dump_opt & METHOD_DUMP_OATCODE)
        Android::OatPrepare();

    if (options.dexpc)
        Android::Prepare();

    return Command::ONCHLD;
}

int MethodCommand::main(int argc, char* const argv[]) {
    art::ArtMethod method = 0x0;
    if (!options.dexpc) {
        method = Utils::atol(argv[options.optind]) & CoreApi::GetVabitsMask();
    } else {
        bool found = false;
        auto search_method = [&](art::ArtMethod& tmp) -> bool {
            art::dex::CodeItem item = tmp.GetCodeItem();
            art::DexFile& dex_file = tmp.GetDexFile();
            if (item.Ptr()) {
                uint64_t start = item.Ptr() + item.code_offset_;
                uint64_t end = start + (item.insns_count_ << 1);
                if (options.dexpc >= start && options.dexpc < end) {
                    found = true;
                    method = tmp;
                    LOGI(ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, method.Ptr());
                }
            }
            return found;
        };

        auto callback = [&](art::mirror::Object& object) -> bool {
            if (!object.IsClass())
                return false;

            art::mirror::Class current = object;
            Android::ForeachArtMethods(current, search_method);
            return found;
        };
        Android::ForeachObjects(callback);
        if (!found) {
            LOGE("Not found ArtMethod include dexpc 0x%" PRIx64 "\n", options.dexpc);
            return 0;
        }
    }

    uint32_t dex_method_idx = method.GetDexMethodIndex();
    if (LIKELY(dex_method_idx != art::dex::kDexNoIndex)) {
        LOGI(ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_LIGHTRED "%s" ANSI_COLOR_RESET " [dex_method_idx=%d]\n", art::PrettyMethodAccessFlags(method.access_flags()).c_str(),
                       method.ColorPrettyMethod().c_str(), dex_method_idx);
        if (options.dump_opt & METHOD_DUMP_DEXCODE)
            Dexdump(method);
    } else {
        LOGI(ANSI_COLOR_LIGHTRED "%s" ANSI_COLOR_RESET "\n", method.GetRuntimeMethodName());
    }

    if (options.dump_opt & METHOD_DUMP_OATCODE)
        Oatdump(method);

    if (options.dump_opt & METHOD_DUMP_BINARY)
        Binarydump(method);

    return 0;
}

void MethodCommand::Dexdump(art::ArtMethod& method) {
    art::dex::CodeItem item = method.GetCodeItem();
    art::DexFile& dex_file = method.GetDexFile();
    if (item.Ptr()) {
        if (options.verbose) {
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

        if (options.instpc >= coderef.Ptr() && options.instpc < endref.Ptr()) {
            coderef = options.instpc;
            endref = options.instpc + art::Dexdump::GetDexInstSize(coderef);
        }

        if (options.count > 0)
            endref = item.Ptr() + item.code_offset_ + (item.insns_count_ << 1);

        coderef.copyRef(item);

        int current = 0;
        while (coderef < endref) {
            LOGI("  %s\n", art::Dexdump::PrettyDexInst(coderef, dex_file).c_str());
            current++;
            if (options.count && current == options.count)
                break;
            coderef.MovePtr(art::Dexdump::GetDexInstSize(coderef));
        }
    } else {
        LOGI(ANSI_COLOR_RED "  NO CODE!\n" ANSI_COLOR_RESET);
    }
}

void MethodCommand::Oatdump(art::ArtMethod& method) {
    uint32_t dex_method_idx = method.GetDexMethodIndex();
    if (!options.pc && LIKELY(dex_method_idx != art::dex::kDexNoIndex)) {
        std::string method_pretty_desc = method.GetDeclaringClass().PrettyDescriptor();
        method_pretty_desc.append(".");
        method_pretty_desc.append(method.GetName());
        options.pc = CoreApi::DlSym(method_pretty_desc.c_str());
    }
    art::OatQuickMethodHeader method_header = 0x0;
    if (method.IsNative()) {
        uint64_t existing_entry_point = method.GetEntryPointFromQuickCompiledCode();
        art::Runtime& runtime = art::Runtime::Current();
        art::ClassLinker& class_linker = runtime.GetClassLinker();
        if (!class_linker.IsQuickGenericJniStub(existing_entry_point) &&
                !class_linker.IsQuickResolutionStub(existing_entry_point)) {
            method_header = art::OatQuickMethodHeader::FromEntryPoint(existing_entry_point);
        }
    } else {
        method_header = method.GetOatQuickMethodHeader(/*method.GetEntryPointFromQuickCompiledCode()*/ options.pc);
    }

    if (options.verbose) {
        if (method_header.Ptr())
            method_header.Dump("");
        if (method_header.Ptr() && method_header.IsOptimized()) {
            art::CodeInfo code_info = art::CodeInfo::Decode(method_header.GetOptimizedCodeInfoPtr());
            code_info.ExtendNumRegister(method);
            code_info.Dump("    ");
        }

        art::QuickMethodFrameInfo frame;
        art::QuickFrame quick = 0x0;
        quick.GetMethodHeader() = method_header.Ptr();
        quick.DirectGetMethod() = method.Ptr();
        quick.SetFramePc(options.pc);
        frame = quick.GetFrameInfo();
        if (method_header.Ptr()) {
            if (!method_header.IsNterpMethodHeader())
                LOGI("  QuickMethodFrameInfo\n");
            else
                LOGI("  NterpFrameInfo\n");
        } else {
            LOGI("  MethodFrameInfo\n");
        }

        LOGI("    frame_size_in_bytes: 0x%x\n", frame.FrameSizeInBytes());
        LOGI("    core_spill_mask: 0x%x %s\n", frame.CoreSpillMask(), art::QuickMethodFrameInfo::PrettySpillMask(frame.CoreSpillMask()).c_str());
        LOGI("    fp_spill_mask: 0x%x %s\n", frame.FpSpillMask(), art::QuickMethodFrameInfo::PrettySpillMask(frame.FpSpillMask()).c_str());
    }

    if (method_header.Ptr()) {
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
                method_header.NativePc2VRegs(start - method_header.GetCodeStart(), vregs, method);
                FrameCommand::ShowJavaFrameRegister("      ", vregs);
                start = stack.native_pc + method_header.GetCodeStart();
                code_size = stack.native_pc;
            }
            capstone::Disassember::Dump("  ", start, method_header.GetCodeSize() - code_size, opt);
        }
    }
}

void MethodCommand::Binarydump(art::ArtMethod& method) {
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
    LOGI("Usage: method [<ART_METHOD>|-s, --search <DEXPC>] [OPTIONE...]\n");
    LOGI("Option:\n");
    LOGI("    --dex-dump            show dalvik byte codes\n");
    LOGI("    -i, --inst <PC>       only dex-dump, show instpc byte code\n");
    LOGI("    -n, --num <NUM>       only dex-dump, show maxline num\n");
    LOGI("    --oat-dump            show oat machine codes\n");
    LOGI("        --pc <PC>         only oat-dump\n");
    LOGI("    -b, --binary          show ArtMethod memory\n");
    LOGI("    -v, --verbaose        show more info\n");
    LOGI("    -s, --search <dexpc>  search all ArtMethod include that dexpc\n");
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
    ENTER();
    LOGI("core-parser> method -s 0000786c528044a8 --dex-dump --inst 0000786c528044a8\n");
    LOGI("[0x6fe055e8]\n");
    LOGI("public static void android.os.Looper.loop() [dex_method_idx=9185]\n");
    LOGI("DEX CODE:\n");
    LOGI("  0x786c528044a8: 4071 23e2 3210           | invoke-static {v0, v1, v2, v3}, boolean android.os.Looper.loopOnce(android.os.Looper, long, int) // method@9186\n");
}
