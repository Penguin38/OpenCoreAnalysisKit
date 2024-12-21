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
#include "command/env.h"
#include "command/cmd_register.h"
#include "common/elf.h"
#include "api/core.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int RegisterCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()) 
        return 0;

    int regs_opt = REGS_DUMP;
    char* command = nullptr;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"set",    required_argument,  0,  's'},
        {"get",    required_argument,  0,  'g'},
    };

    while ((opt = getopt_long(argc, argv, "s:g:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                regs_opt = REGS_SET;
                command = optarg;
                break;
            case 'g':
                regs_opt = REGS_GET;
                command = optarg;
                break;
        }
    }

    int pid = Env::CurrentPid();
    if (optind < argc) pid = std::atoi(argv[optind]);

    ThreadApi* thread = CoreApi::FindThread(pid);

    if (!thread) {
        LOGE("Invalid tid %d\n", pid);
        return 0;
    }

    switch(regs_opt) {
        case REGS_DUMP:
            thread->RegisterDump("    ");
            break;
        case REGS_SET:
            thread->RegisterSet(command);
            break;
        case REGS_GET:
            LOGI("0x%" PRIx64 "\n", thread->RegisterGet(command));
            break;
    }
    return 0;
}

void RegisterCommand::usage() {
    LOGI("Usage: register|regs [TID] [OPTION...]\n");
    LOGI("Option:\n");
    LOGI("    -s, --set <REGS>=<VALUE>    overwrite register value\n");
    LOGI("    -g, --get <REGS>            get target register value\n");
    ENTER();
    LOGI("core-parser> regs --set rip=0x00000074e414e2a0\n");
    LOGI("New note overlay [7d6d8, 104073)\n");
    ENTER();
    LOGI("core-parser> regs --set ucontext[=<ADDR>]\n");
}
