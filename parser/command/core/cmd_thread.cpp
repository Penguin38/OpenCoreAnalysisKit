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
#include "common/elf.h"
#include "command/env.h"
#include "command/core/cmd_thread.h"
#include "base/utils.h"
#include "api/core.h"
#include "android.h"
#include "runtime/thread_list.h"
#include <unistd.h>
#include <getopt.h>
#include <string>
#include <vector>

int ThreadCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return Command::FINISH;

    if (!(argc > 1)) {
        LOGI("Current thread is %d\n", Env::CurrentPid());
        return Command::FINISH;
    }

    options.native = false;
    options.java = false;
    options.dump_all = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"native",    no_argument,       0,  'n'},
#if defined(__AOSP_PARSER__)
        {"java",      no_argument,       0,  'j'},
        {"all",       no_argument,       0,  'a'},
        {0,           0,                 0,   0 },
#endif
    };

    while ((opt = getopt_long(argc, (char* const*)argv, "nja",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'n':
                options.native = true;
                options.java = false;
                break;
            case 'j':
                options.native = false;
                options.java = true;
                break;
            case 'a':
                options.native = false;
                options.java = true;
                options.dump_all = true;
                break;
        }
    }
    options.optind = optind;

#if defined(__AOSP_PARSER__)
    if (options.java)
        Android::Prepare();
#endif

    return !(options.native ^ options.java) ? Command::CONTINUE : Command::ONCHLD;
}

int ThreadCommand::main(int argc, char* const argv[]) {
    if (!(options.native ^ options.java)) {
        if (options.optind < argc) {
            int current_pid = std::atoi(argv[options.optind]);
            Env::SetCurrentPid(current_pid);
        }
        LOGI("Current thread is %d\n", Env::CurrentPid());
    } else {
        if (options.native) {
            int index = 1;
            int machine = CoreApi::GetMachine();
            LOGI(ANSI_COLOR_LIGHTRED " ID     TARGET TID        FRAME\n" ANSI_COLOR_RESET);
            auto callback = [&](ThreadApi *api) -> bool {
                uint64_t frame_pc = api->GetFramePC();
                File* file = CoreApi::FindFile(frame_pc);
                LOGI("%s%-4d   Thread " ANSI_COLOR_YELLOW "%-10d " ANSI_COLOR_CYAN "0x%" PRIx64 "  " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET,
                        api->pid() == Env::CurrentPid() ? "*" : " ",
                        index, api->pid(), frame_pc, file? file->name().c_str() : "");
                ++index;
                return false;
            };
            CoreApi::ForeachThread(callback);
        } else {
#if defined(__AOSP_PARSER__)
            if (!Android::IsSdkReady())
                return 0;

            std::vector<int> threads;
            auto callback = [&](ThreadApi *api) -> bool {
                threads.push_back(api->pid());
                return false;
            };
            CoreApi::ForeachThread(callback);

            LOGI(ANSI_COLOR_LIGHTRED " ID   TID    STATUS                          ADDRESS              NAME\n" ANSI_COLOR_RESET);
            art::ThreadList& thread_list = art::Runtime::Current().GetThreadList();
            for (const auto& thread : thread_list.GetList()) {
                int tid = thread->GetTid();
                auto it = std::find(threads.begin(), threads.end(), tid);
                LOGI("%s%-4d " ANSI_COLOR_LIGHTYELLOW "%-6d " ANSI_COLOR_LIGHTCYAN "%-31s "
                               ANSI_COLOR_RESET ANSI_COLOR_LIGHTMAGENTA "0x%-16" PRIx64 "" ANSI_COLOR_RESET
                               "   \"" ANSI_COLOR_LIGHTRED "%s" ANSI_COLOR_RESET "\" %s\n",
                        tid == Env::CurrentPid() ? "*" : " ",
                        thread->GetThreadId(), tid, thread->GetStateDescriptor(), thread->Ptr(),
                        thread->GetName().c_str(), it != threads.end() ? "" : "(NOT EXIST THREAD)");
                if (it != threads.end()) threads.erase(it);
            }

            if (options.dump_all) {
                for (int pid : threads) {
                    LOGI("%s---  " ANSI_COLOR_YELLOW "%-6d " ANSI_COLOR_CYAN "NotAttachJVM\n" ANSI_COLOR_RESET , pid == Env::CurrentPid() ? "*" : " ", pid);
                }
            }
#endif
        }
    }

    return 0;
}

void ThreadCommand::usage() {
    LOGI("Usage: thread [TID] [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -n, --native    show local threads\n");
#if defined(__AOSP_PARSER__)
    LOGI("    -j, --java      show jvm threads\n");
    LOGI("    -a, --all       show all thread\n");
    ENTER();
    LOGI("core-parser> thread -a\n");
    LOGI(" ID   TID    STATUS                          NAME\n");
    LOGI("*1    6118   Runnable                        \"main\"\n");
    LOGI(" 2    6125   Native                          \"Runtime worker thread 0\"\n");
    LOGI(" 3    6128   Native                          \"Runtime worker thread 3\"\n");
    LOGI(" 4    6126   Native                          \"Runtime worker thread 1\"\n");
    LOGI(" 5    6129   WaitingInMainSignalCatcherLoop  \"Signal Catcher\"\n");
    LOGI(" 6    6127   Native                          \"Runtime worker thread 2\"\n");
    LOGI("...\n");
#endif
    ENTER();
    LOGI("core-parser> thread -n\n");
    LOGI(" ID     TARGET TID        FRAME\n");
    LOGI("*1      Thread 6118       0x79185c88945f  /apex/com.android.art/lib64/libart.so\n");
    LOGI(" 2      Thread 6125       0x791aef6632a8  /apex/com.android.runtime/lib64/bionic/libc.so\n");
    LOGI(" 3      Thread 6126       0x791aef6632a8  /apex/com.android.runtime/lib64/bionic/libc.so\n");
    LOGI(" 4      Thread 6127       0x791aef6632a8  /apex/com.android.runtime/lib64/bionic/libc.so\n");
    LOGI("...\n");
    ENTER();
    LOGI("core-parser> thread\n");
    LOGI("Current thread is 6118\n");
    ENTER();
    LOGI("core-parser> thread 6133\n");
    LOGI("Current thread is 6133\n");
}
