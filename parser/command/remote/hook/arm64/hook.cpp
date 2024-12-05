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
#include "common/bit.h"
#include "command/env.h"
#include "command/remote/opencore/opencore.h"
#include "command/remote/hook/arm64/hook.h"
#include "command/remote/cmd_remote.h"
#include <dlfcn.h>
#include <string.h>

namespace arm64 {

bool Hook::InjectLibrary(const char* library) {
    if (!CoreApi::IsRemote())
        return false;

    if (Env::CurrentRemotePid() != Pid())
        return false;

    if (!library || !strlen(library))
        return false;

    LOGI("arm64: hook inject \"%s\"\n", library);

    uint64_t dlopen_load = CoreApi::DlSym("dlopen");
    if (!dlopen_load)
        return false;
    LOGI("arm64: hook found \"dlopen\" address: 0x%lx\n", dlopen_load);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    // Backup target process context
    if (!LoadContext(&ori_regs))
        return false;

    LOGI("arm64: target process current sp: 0x%lx\n", ori_regs.sp);
    pt_regs call_regs;
    memcpy(&call_regs, &ori_regs, sizeof(pt_regs));

    call_regs.sp = ori_regs.sp - RoundUp(strlen(library) + 1, 0x10);
    RemoteCommand::Write(Pid(), call_regs.sp, (void *)library, strlen(library) + 1);

    call_regs.regs[0]  = call_regs.sp;
    call_regs.regs[1]  = RTLD_NOW;
    call_regs.regs[30] = 0x0;
    call_regs.pc       = dlopen_load;

    if (!StoreContext(&call_regs))
        return false;

    LOGI("arm64: call dlopen(0x%lx \"%s\", 0x%lx)\n",
            call_regs.regs[0], library, call_regs.regs[1]);
    if (!Continue())
        return false;

    // Get return retval
    pt_regs retval_regs;
    LoadContext(&retval_regs);
    LOGI("arm64: return 0x%lx\n", retval_regs.regs[0]);

    // Restore origin context
    StoreContext(&ori_regs);
    return true;
}

} // namespace arm64
