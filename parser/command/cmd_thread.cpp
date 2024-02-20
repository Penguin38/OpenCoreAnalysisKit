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
#include "common/elf.h"
#include "command/env.h"
#include "command/cmd_thread.h"
#include "base/utils.h"
#include "arm64/thread_info.h"
#include "arm/thread_info.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>
#include <string>

int ThreadCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    if (!argc) {
        LOGI("Current thread is %d\n", Env::CurrentPid());
        return 0;
    }

    int nargc = argc + 1;
    const char* nargv[16];
    for (int i = 0; i < 15; ++i) {
        nargv[i + 1] = argv[i];
    }

    std::string current = std::to_string(Env::CurrentPid());
    nargv[0] = current.c_str();

    int opt;
    int option_index = 0;
    bool native = false;
    bool java = false;
    static struct option long_options[] = {
        {"native",    no_argument,       0,  'n'},
        {"java",      no_argument,       0,  'j'},
    };

    while ((opt = getopt_long(nargc, (char* const*)nargv, "nj",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'n':
                native = true;
                java = false;
                break;
            case 'j':
                native = false;
                java = true;
                break;
        }
    }

    // reset
    optind = 0;

    if (!(native ^ java)) {
        int current_pid = atoi(argv[0]);
        Env::SetCurrentPid(current_pid);
        LOGI("Current thread is %d\n", Env::CurrentPid());
    } else {
        if (native) {
            int index = 1;
            int machine = CoreApi::GetMachine();
            uint64_t frame_pc = 0x0;
            LOGI(" Id   Target Id         Frame\n");
            auto callback = [&](ThreadApi *api) -> bool {
                switch(machine) {
                    case EM_AARCH64: {
                        arm64::ThreadInfo* info = reinterpret_cast<arm64::ThreadInfo*>(api);
                        frame_pc = info->GetRegs().pc;
                    } break;
                    case EM_ARM: {
                        arm::ThreadInfo* info = reinterpret_cast<arm::ThreadInfo*>(api);
                        frame_pc = info->GetRegs().pc;
                    } break;
                    case EM_RISCV:
                        break;
                    case EM_X86_64:
                        break;
                    case EM_386:
                        break;
                }

                LOGI("%s%-3d  Thread %-10d 0x%lx\n",
                        api->pid() == Env::CurrentPid() ? "*" : " ",
                        index, api->pid(), frame_pc);
                ++index;
                return false;
            };
            CoreApi::ForeachThread(callback);
        } else {

        }
    }
    return 0;
}

void ThreadCommand::usage() {
    LOGI("Usage: thread [tid] [--native|-n] [--java|-j]");
}
