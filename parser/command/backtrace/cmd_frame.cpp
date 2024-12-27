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
#include "api/core.h"
#include "api/unwind.h"
#include "base/utils.h"
#include "common/elf.h"
#include "command/env.h"
#include "command/cmd_dex.h"
#include "command/backtrace/cmd_frame.h"
#include "command/backtrace/cmd_backtrace.h"
#include "runtime/thread_list.h"
#include "runtime/stack.h"
#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
#include "common/disassemble/capstone.h"
#include <unistd.h>
#include <getopt.h>

int FrameCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"java",    no_argument,       0,  'j'},
        {"native",  no_argument,       0,  'n'},
        {"all",     no_argument,       0,  'a'},
    };

    java = false;
    dump_all = false;
    if (Android::IsSdkReady() && art::Runtime::Current().Ptr()) {
        art::Thread* current = art::Runtime::Current().GetThreadList().FindThreadByTid(Env::CurrentPid());
        if (current && !current->StackEmpty())
            java = true;
    }

    while ((opt = getopt_long(argc, (char* const*)argv, "jna",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'j':
                java = true;
                break;
            case 'n':
                java = false;
                break;
            case 'a':
                dump_all = true;
                break;
        }
    }

    int number = 0;
    if (optind < argc) number = std::atoi(argv[optind]);

#if defined(__AOSP_PARSER__)
    if (dump_all) {
        ShowNativeFrameInfo(number);
        ShowJavaFrameInfo(number);
    } else {
        if (java) {
            ShowJavaFrameInfo(number);
        } else {
            ShowNativeFrameInfo(number);
        }
    }
#else
    ShowNativeFrameInfo(number);
#endif
    return 0;
}

struct NearAsm {
    int length;
    int count;
};

static NearAsm NearAsmLength() {
    NearAsm near_asm;
    int machine = CoreApi::GetMachine();
    switch (machine) {
        case EM_386:
            near_asm.length = 0x20;
            near_asm.count = 12;
            break;
        case EM_X86_64:
            near_asm.length = 0x20;
            near_asm.count = 12;
            break;
        case EM_ARM:
            near_asm.length = 0x20;
            near_asm.count = 16;
            break;
        case EM_AARCH64:
            near_asm.length = 0x24;
            near_asm.count = 12;
            break;
        case EM_RISCV:
            near_asm.length = 0x24;
            near_asm.count = 12;
            break;
    }
    return near_asm;
}

void FrameCommand::ShowJavaFrameInfo(int number) {
    art::Thread* current = nullptr;
    int pid = Env::CurrentPid();
    if (Android::IsSdkReady() && art::Runtime::Current().Ptr()) {
        current = art::Runtime::Current().GetThreadList().FindThreadByTid(pid);
    }

    if (!current)
        return;

    art::StackVisitor visitor(current, art::StackVisitor::StackWalkKind::kSkipInlinedFrames);
    visitor.WalkStack();

    if (number > visitor.GetJavaFrames().size() - 1 && !dump_all)
        return;

    std::string format = BacktraceCommand::FormatJavaFrame("  ", visitor.GetJavaFrames().size());
    uint32_t frameid = 0;
    for (const auto& java_frame : visitor.GetJavaFrames()) {
        if (dump_all || number == frameid) {
            art::ArtMethod& method = java_frame->GetMethod();
            art::ShadowFrame& shadow_frame = java_frame->GetShadowFrame();
            art::QuickFrame& quick_frame = java_frame->GetQuickFrame();
            art::DexFile& dex_file = method.GetDexFile();
            std::string location = DexCommand::DexFileLocation(dex_file, false);
            uint64_t dex_pc_ptr = java_frame->GetDexPcPtr();
            LOGI(format.c_str(), frameid, dex_pc_ptr, method.ColorPrettyMethodOnlyNP().c_str());
            LOGI("  {\n");
            LOGI("      Location: " ANSI_COLOR_LIGHTGREEN "%s\n" ANSI_COLOR_RESET, location.c_str());
            if (Android::Sdk() >= Android::P)
                LOGD("      IsCompact: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, dex_file.IsCompactDexFile() ? "true" : "false");
            LOGI("      art::ArtMethod: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, method.Ptr());
            if (dex_pc_ptr) LOGI("      dex_pc_ptr: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, dex_pc_ptr);
            if (shadow_frame.Ptr()) LOGI("      shadow_frame: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, shadow_frame.Ptr());
            if (quick_frame.Ptr()) {
                LOGI("      quick_frame: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, quick_frame.Ptr());
                art::OatQuickMethodHeader& method_header = java_frame->GetMethodHeader();
                if (java_frame->GetFramePc()) LOGI("      frame_pc: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, java_frame->GetFramePc());
                LOGI("      method_header: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, method_header.Ptr());
            }

            if (dex_pc_ptr) {
                LOGI(ANSI_COLOR_LIGHTRED "\n      DEX CODE:\n" ANSI_COLOR_RESET);
                art::dex::CodeItem item = method.GetCodeItem();
                api::MemoryRef startref = item.Ptr() + item.code_offset_;
                api::MemoryRef coderef = dex_pc_ptr;
                startref.copyRef(item);

                while (startref.Ptr() <= (coderef.Ptr() - 0xc)) {
                    startref.MovePtr(art::Dexdump::GetDexInstSize(startref));
                }

                while (startref <= coderef) {
                    LOGI("      %s\n", art::Dexdump::PrettyDexInst(startref, dex_file).c_str());
                    startref.MovePtr(art::Dexdump::GetDexInstSize(startref));
                }
                ShowJavaFrameRegister("      ", java_frame->GetVRegs(), quick_frame);
            }

            if (java_frame->GetFramePc()) {
                LOGI(ANSI_COLOR_RED "\n      OAT CODE:\n" ANSI_COLOR_RESET);
                art::OatQuickMethodHeader& method_header = java_frame->GetMethodHeader();
                NearAsm near_asm = NearAsmLength();
                capstone::Disassember::Option opt((java_frame->GetFramePc() & CoreApi::GetVabitsMask()) - near_asm.length, near_asm.count);
                if (CoreApi::GetMachine() == EM_ARM) {
                    opt.SetArchMode(capstone::Disassember::Option::ARCH_ARM, capstone::Disassember::Option::MODE_THUMB);
                }
                capstone::Disassember::Dump("      ", method_header.GetCodeStart(), method_header.GetCodeSize(), opt);
                art::QuickFrame& prev_quick_frame = java_frame->GetPrevQuickFrame();
                if (prev_quick_frame.Ptr()) {
                    art::QuickMethodFrameInfo frame = prev_quick_frame.GetFrameInfo();
                    frame.DumpCoreSpill("      ", prev_quick_frame.Ptr());
                }
            }
            LOGI("  }\n");
        }
        ++frameid;
    }
}

void FrameCommand::ShowJavaFrameRegister(const char* prefix,
                                         std::map<uint32_t, art::DexRegisterInfo>& vregs,
                                         api::MemoryRef& frame) {
    uint32_t vregs_size = vregs.size();
    if (vregs_size) {
        std::string sb;
        sb.append(prefix).append("{\n");
        int num = 0;
        for (const auto& vreg : vregs) {
            uint32_t kind = vreg.second.Kind();
            uint32_t value = vreg.second.PackedValue();

            if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInvalid)
                    || kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kNone)) {
                vregs_size--;
                continue;
            }

            if (!(num % 4))
                sb.append(prefix).append("    ");

            sb.append("v");
            sb.append(std::to_string(vreg.first));
            sb.append(" = ");

            char valuehex[16];
            if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kConstant)) {
                snprintf(valuehex, sizeof(valuehex), "0x%08x", value);
                sb.append(valuehex);
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInRegister)) {
                sb.append(art::QuickFrame::RegisterDesc(value, true));
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInRegisterHigh)) {
                sb.append(art::QuickFrame::RegisterDesc(value, false));
                sb.append("/hi");
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInFpuRegister)) {
                sb.append("f");
                sb.append(std::to_string(value));
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInFpuRegisterHigh)) {
                sb.append("f");
                sb.append(std::to_string(value));
                sb.append("/hi");
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInStack)) {
                value *= art::kFrameSlotSize;
                if (frame.Ptr()) {
                    snprintf(valuehex, sizeof(valuehex), "0x%08x", frame.value32Of(value));
                    sb.append(valuehex);
                } else {
                    sb.append("[sp+#");
                    sb.append(std::to_string(value));
                    sb.append("]");
                }
            }
            if (num < vregs_size - 1) {
                sb.append("    ");
            }
            num++;
            if (!(num % 4))
                sb.append("\n");
        }

        if (vregs_size && (vregs_size % 4))
            sb.append("\n");
        sb.append(prefix).append("}\n");
        LOGI("%s", sb.c_str());
    }
}

void FrameCommand::ShowNativeFrameInfo(int number) {
    int pid = Env::CurrentPid();
    ThreadApi* api = CoreApi::FindThread(pid);
    std::unique_ptr<api::UnwindStack> unwind_stack = api::UnwindStack::MakeUnwindStack(api);
    if (unwind_stack) {
        unwind_stack->WalkStack();
        std::string format = BacktraceCommand::FormatNativeFrame("  ", unwind_stack->GetNativeFrames().size());
        uint32_t frameid = 0;
        for (const auto& native_frame : unwind_stack->GetNativeFrames()) {
            if (dump_all || frameid == number) {
                std::string method_desc = native_frame->GetMethodName();
                uint64_t cloc_pc = native_frame->GetFramePc() & CoreApi::GetVabitsMask();
                uint64_t offset = cloc_pc - native_frame->GetMethodOffset();
                if (offset && native_frame->GetMethodOffset())
                    method_desc.append("+").append(Utils::ToHex(offset));

                if (!method_desc.length() && native_frame->GetLinkMap()
                        && native_frame->GetLinkMap()->begin()) {
                    method_desc.append(native_frame->GetLibrary());
                    method_desc.append("+").append(Utils::ToHex(offset-native_frame->GetLinkMap()->begin()));
                }
                LOGI(format.c_str(), frameid, native_frame->GetFramePc(), method_desc.c_str());
                LOGI("  {\n");
                LOGI("      library: " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, native_frame->GetLibrary().c_str());
                LOGI("      symbol: " ANSI_COLOR_YELLOW "%s\n" ANSI_COLOR_RESET, native_frame->GetMethodSymbol().c_str());
                LOGI("      frame_fp: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, native_frame->GetFrameFp());
                LOGI("      frame_pc: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, native_frame->GetFramePc());
                if (CoreApi::GetMachine() == EM_ARM)
                    LOGI("      thumb: %s\n", native_frame->IsThumbMode() ? "true" : "false");

                if (native_frame->GetFramePc()) {
                    LOGI(ANSI_COLOR_RED "\n      ASM CODE:\n" ANSI_COLOR_RESET);
                    NearAsm near_asm = NearAsmLength();
                    capstone::Disassember::Option opt(cloc_pc - near_asm.length, near_asm.count);
                    if (CoreApi::GetMachine() == EM_ARM) {
                        if (native_frame->IsThumbMode()) {
                            opt.SetArchMode(capstone::Disassember::Option::ARCH_ARM, capstone::Disassember::Option::MODE_THUMB);
                        } else {
                            opt.SetArchMode(capstone::Disassember::Option::ARCH_ARM, capstone::Disassember::Option::MODE_ARM);
                        }
                    }
                    if (native_frame->GetMethodOffset()) {
                        capstone::Disassember::Dump("      ", native_frame->GetMethodOffset(), native_frame->GetMethodSize(), opt);
                    } else {
                        uint64_t near = 0x0;
                        if (CoreApi::GetMachine() == EM_AARCH64
                                || (CoreApi::GetMachine() == EM_ARM && !native_frame->IsThumbMode()))
                            near = near_asm.length - 4;
                        capstone::Disassember::Dump("      ", cloc_pc - near, near_asm.length, opt);
                    }
                }
                LOGI("  }\n");
            }
            ++frameid;
        }
    }
}

void FrameCommand::usage() {
    LOGI("Usage: frame|f <NUM> [OPTION..]\n");
    LOGI("Option:\n");
    LOGI("    -j, --java       show java frame info (default)\n");
    LOGI("    -n, --native     show native frame info\n");
    LOGI("    -a, --all        show all frame info\n");
    ENTER();
    LOGI("core-parser> f 6 -j\n");
    LOGI("  JavaKt: #6  000079185a704980  com.android.internal.os.ZygoteInit.main(java.lang.String[])\n");
    LOGI("  {\n");
    LOGI("      Location: /system/framework/framework.jar!classes3.dex\n");
    LOGI("      art::ArtMethod: 0x70b509c0\n");
    LOGI("      dex_pc_ptr: 0x79185a704980\n");
    LOGI("      quick_frame: 0x7ffc73acdc70\n");
    LOGI("      frame_pc: 0x71907dd5\n");
    LOGI("      method_header: 0x719075d8\n");
    ENTER();
    LOGI("      DEX CODE:\n");
    LOGI("      0x79185a704976: 106e c34e 0002           | invoke-virtual {v2}, void com.android.internal.os.ZygoteServer.closeServerSocket() // method@49998\n");
    LOGI("      0x79185a70497c: 0038 0005                | if-eqz v0, 0x79185a704986 //+5\n");
    LOGI("      0x79185a704980: 1072 ebbe 0000           | invoke-interface {v0}, void java.lang.Runnable.run() // method@60350\n");
    ENTER();
    LOGI("      OAT CODE:\n");
    LOGI("      0x71907db5:                   2057ff | call qword ptr [rdi + 0x20]\n");
    LOGI("      0x71907db8:                     db85 | test ebx, ebx\n");
    LOGI("      0x71907dba:             00000018840f | je 0x71907dd8\n");
    LOGI("      0x71907dc0:                   de8948 | mov rsi, rbx\n");
    LOGI("      0x71907dc3:                     3e8b | mov edi, dword ptr [rsi]\n");
    LOGI("      0x71907dc5:               0000ebbeb8 | mov eax, 0xebbe\n");
    LOGI("      0x71907dca:           00000080bf8b48 | mov rdi, qword ptr [rdi + 0x80]\n");
    LOGI("      0x71907dd1:                 207f8b48 | mov rdi, qword ptr [rdi + 0x20]\n");
    LOGI("      0x71907dd5:                   2057ff | call qword ptr [rdi + 0x20]\n");
    LOGI("      0x71907dd8:                 78c48348 | add rsp, 0x78\n");
    LOGI("      0x71907ddc:                       5b | pop rbx\n");
    LOGI("      0x71907ddd:                       5d | pop rbp\n");
    LOGI("      {\n");
    LOGI("          rcx = 0x0000000012c5a298    rdx = 0x0000000013040218    rbx = 0x0000000000000004    rbp = 0x0000000012c5f340    \n");
    LOGI("          rsi = 0x0000000000000000    r8 = 0x000079185a9cb7f1    r9 = 0x0000000071bd5fb8    r12 = 0x00000000705d9d70    \n");
    LOGI("          r13 = 0x0000000071be62f0    r14 = 0x0000000000000000    r15 = 0x0000000071907dd8\n");
    LOGI("      }\n");
    LOGI("  }\n");
}
