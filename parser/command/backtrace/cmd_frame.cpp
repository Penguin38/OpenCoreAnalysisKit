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
#include "command/env.h"
#include "command/backtrace/cmd_frame.h"
#include "command/backtrace/cmd_backtrace.h"
#include "runtime/thread_list.h"
#include "runtime/stack.h"
#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
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
    };

    java = false;
    if (Android::IsSdkReady() && art::Runtime::Current().Ptr()) {
        if (art::Runtime::Current().GetThreadList().Contains(Env::CurrentPid()))
            java = true;
    }
    while ((opt = getopt_long(argc, (char* const*)argv, "jn",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'j':
                java = true;
                break;
            case 'n':
                java = false;
                break;
        }
    }

    int number = 0;
    if (optind < argc) number = atoi(argv[optind]);

#if defined(__AOSP_PARSER__)
    if (java) {
        ShowJavaFrameInfo(number);
    } else {
        ShowNativeFrameInfo(number);
    }
#endif
    return 0;
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

    if (number > visitor.GetJavaFrames().size() - 1)
        return;

    std::string format = BacktraceCommand::FormatJavaFrame("  ", visitor.GetJavaFrames().size());
    uint32_t frameid = 0;
    for (const auto& java_frame : visitor.GetJavaFrames()) {
        if (number == frameid) {
            art::ArtMethod& method = java_frame->GetMethod();
            art::ShadowFrame& shadow_frame = java_frame->GetShadowFrame();
            art::QuickFrame& quick_frame = java_frame->GetQuickFrame();
            art::DexFile& dex_file = method.GetDexFile();
            uint64_t dex_pc_ptr = java_frame->GetDexPcPtr();
            LOGI(format.c_str(), frameid, dex_pc_ptr, method.PrettyMethodOnlyNP().c_str());
            LOGI("  {\n");
            LOGI("      art::ArtMethod: 0x%lx\n", method.Ptr());
            LOGI("      shadow_frame: 0x%lx\n", shadow_frame.Ptr());
            LOGI("      quick_frame: 0x%lx\n", quick_frame.Ptr());
            LOGI("      dex_pc_ptr: 0x%lx\n", dex_pc_ptr);
            if (quick_frame.Ptr()) {
                art::OatQuickMethodHeader& method_header = java_frame->GetMethodHeader();
                LOGI("      frame_pc: 0x%lx\n", java_frame->GetFramePc());
                LOGI("      method_header: 0x%lx\n", method_header.Ptr());
            }

            if (dex_pc_ptr) {
                LOGI("\n      DEX CODE:\n");
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

            art::QuickFrame& prev_quick_frame = java_frame->GetPrevQuickFrame();
            if (prev_quick_frame.Ptr()) {
                LOGI("\n      OAT CODE:\n");
                art::QuickMethodFrameInfo frame = prev_quick_frame.GetFrameInfo();
                frame.DumpCoreSpill("      ", prev_quick_frame.Ptr());
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

            char valuehex[11];
            if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kConstant)) {
                sprintf(valuehex, "0x%08x", value);
                sb.append(valuehex);
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInRegister)) {
                sb.append("r");
                sb.append(std::to_string(value));
            } else if (kind == static_cast<uint32_t>(art::DexRegisterInfo::Kind::kInRegisterHigh)) {
                sb.append("r");
                sb.append(std::to_string(value));
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
                    sprintf(valuehex, "0x%08x", frame.value32Of(value));
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
            if (frameid == number) {
                std::string method_desc = native_frame->GetMethodName();
                if (native_frame->GetMethodOffset()) method_desc.append("+").append(Utils::ToHex(native_frame->GetMethodOffset()));
                LOGI(format.c_str(), frameid, native_frame->GetFramePc(), method_desc.c_str());
                LOGI("  {\n");
                LOGI("      library: %s\n", native_frame->GetLibrary().c_str());
                LOGI("      frame_fp: 0x%lx\n", native_frame->GetFrameFp());
                LOGI("      frame_pc: 0x%lx\n", native_frame->GetFramePc());
                LOGI("  }\n");
            }
            ++frameid;
        }
    }
}

void FrameCommand::usage() {
    LOGI("Usage: frame|f <NUM> [option..]\n");
    LOGI("Option:\n");
    LOGI("    --java|-j: show java frame info. (Default)\n");
    LOGI("    --native|-n: show native frame info.\n");
}
