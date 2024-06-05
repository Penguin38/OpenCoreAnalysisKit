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
#include "command/env.h"
#include "command/backtrace/cmd_frame.h"
#include "command/backtrace/cmd_backtrace.h"
#include <unistd.h>
#include <getopt.h>

#if defined(__AOSP_PARSER__)
#include "runtime/thread_list.h"
#include "runtime/stack.h"
#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
#endif

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

    java = true; // default
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
    }
#endif
    return 0;
}

void FrameCommand::ShowJavaFrameInfo(int number) {
    art::Thread* current = nullptr;
    int pid = Env::CurrentPid();
    if (Android::IsSdkReady()) {
        art::ThreadList& thread_list = art::Runtime::Current().GetThreadList();
        for (const auto& thread : thread_list.GetList()) {
            int tid = thread->GetTid();
            if (tid == pid) {
                current = thread.get();
                break;
            }
        }
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
            api::MemoryRef& quick_frame = java_frame->GetQuickFrame();
            art::DexFile& dex_file = method.GetDexFile();
            LOGI(format.c_str(), frameid, method.PrettyMethodOnlyNP().c_str());
            LOGI("  {\n");
            LOGI("      art::ArtMethod: 0x%lx\n", method.Ptr());
            LOGI("      shadow_frame: 0x%lx\n", shadow_frame.Ptr());
            LOGI("      quick_frame: 0x%lx\n", quick_frame.Ptr());
            if (shadow_frame.Ptr() && shadow_frame.GetDexPcPtr()) {
                LOGI("\n");
                api::MemoryRef coderef = shadow_frame.GetDexPcPtr();
                LOGI("      %s\n", art::Dexdump::PrettyDexInst(coderef, dex_file).c_str());
                ShowJavaFrameRegister("      ", shadow_frame.GetVRegs());
            }
            LOGI("  }\n");
        }
        ++frameid;
    }
}

void FrameCommand::ShowJavaFrameRegister(const char* prefix, std::vector<uint32_t>& vregs) {
    uint32_t vregs_size = vregs.size();
    uint32_t vregs_line = vregs_size / 4;
    uint32_t vregs_mod = vregs_size % 4;
    if (vregs_size) {
        LOGI("%s{\n", prefix);
        for (int i = 0; i < vregs_line; i++) {
            char value[256];
            sprintf(value, "%s    v%d = 0x%08x    v%d = 0x%08x    v%d = 0x%08x    v%d = 0x%08x",
                    prefix, 4 * i, vregs[4 * i], 4 * i + 1, vregs[4 * i + 1],
                    4 * i + 2, vregs[4 * i + 2], 4 * i + 3, vregs[4 * i + 3]);
            LOGI("%s\n", value);
        }

        if (vregs_mod) {
            LOGI("%s", prefix);
            for (int i = 0; i < vregs_mod; i++) {
                char value[32];
                sprintf(value, "    v%d = 0x%08x", 4 * vregs_line + i, vregs[4 * vregs_line + i]);
                LOGI("%s", value);
            }
            LOGI("\n");
        }
        LOGI("%s}\n", prefix);
    }
}

void FrameCommand::usage() {
}
