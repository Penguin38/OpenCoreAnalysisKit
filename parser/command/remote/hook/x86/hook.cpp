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
#include "api/core.h"
#include "common/bit.h"
#include "command/env.h"
#include "command/remote/opencore/opencore.h"
#include "command/remote/hook/x86/hook.h"
#include "command/remote/cmd_remote.h"
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>

namespace x86 {

bool Hook::InjectLibrary(const char* library) {
    if (!library || !strlen(library))
        return false;

    if (!InitEnv())
        return false;

    LOGI("x86: hook inject \"%s\"\n", library);

    uint64_t dlopen_load = CoreApi::DlSym("dlopen");
    if (!dlopen_load)
        return false;
    LOGI("x86: hook found \"dlopen\" address: 0x%" PRIx64 "\n", dlopen_load);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    // Backup target process context
    if (!LoadContext(&ori_regs))
        return false;

    LOGI("x86: target process current esp: 0x%x\n", ori_regs.esp);
    pt_regs call_regs;
    memcpy(&call_regs, &ori_regs, sizeof(pt_regs));

    call_regs.esp = ori_regs.esp - RoundUp(strlen(library) + 1, 4);
    RemoteCommand::Write(Pid(), call_regs.esp, (void *)library, strlen(library) + 1);

    call_regs.edi = call_regs.esp;
    call_regs.esi = RTLD_NOW;
    call_regs.eip = dlopen_load;

    if (!StoreContext(&call_regs))
        return false;

    if (!Continue())
        return false;

    // Get return retval
    pt_regs retval_regs;
    LoadContext(&retval_regs);
    LOGI("x86: return 0x%x\n", retval_regs.eax);

    // Restore origin context
    StoreContext(&ori_regs);
    return true;
}

} // namespace x86
