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
#include "arm64/thread_info.h"
#include "arm/thread_info.h"
#include "x64/thread_info.h"
#include "x86/thread_info.h"
#include "api/core.h"
#include <stdlib.h>

int RegisterCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()) 
        return 0;

    ThreadApi* thread = nullptr;
    int pid = Env::CurrentPid();
    if (argc > 1) pid = atoi(argv[1]);
    thread = CoreApi::FindThread(pid);

    if (!thread) {
        LOGE("ERROR: Invalid tid %d\n", pid);
        return 0;
    }

    int machine = CoreApi::GetMachine();
    switch(machine) {
        case EM_AARCH64: {
            arm64::ThreadInfo* info = reinterpret_cast<arm64::ThreadInfo*>(thread);
            info->RegisterDump("    ");
        } break;
        case EM_ARM: {
            arm::ThreadInfo* info = reinterpret_cast<arm::ThreadInfo*>(thread);
            info->RegisterDump("    ");
        } break;
        case EM_RISCV:
            break;
        case EM_X86_64: {
            x64::ThreadInfo* info = reinterpret_cast<x64::ThreadInfo*>(thread);
            info->RegisterDump("    ");
        } break;
        case EM_386: {
            x86::ThreadInfo* info = reinterpret_cast<x86::ThreadInfo*>(thread);
            info->RegisterDump("    ");
        } break;
    }
    return 0;
}

void RegisterCommand::usage() {
    LOGI("Usage: register [tid]");
}
