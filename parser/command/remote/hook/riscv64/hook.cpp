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
#include "command/remote/hook/riscv64/hook.h"
#include "command/remote/cmd_remote.h"
#include "command/remote/fakecore/process.h"
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>

namespace riscv64 {

bool Hook::InjectLibrary(const char* library) {
    if (!CoreApi::IsRemote() || Env::CurrentRemotePid() != Pid()) {
        std::unique_ptr<FakeCore::Stream> process =
                std::make_unique<fakecore::Process>(Pid());
        std::unique_ptr<FakeCore> impl = FakeCore::Make(process);
        if (!impl)
            return false;

        impl->InitPageSize(sysconf(_SC_PAGE_SIZE));
        impl->InitMask(FakeCore::NO_FAKE_REBUILD);
        impl->execute(nullptr);
    }

    if (!library || !strlen(library))
        return false;

    LOGI("riscv64: hook inject \"%s\"\n", library);

    uint64_t dlopen_load = CoreApi::DlSym("dlopen");
    if (!dlopen_load)
        return false;
    LOGI("riscv64: hook found \"dlopen\" address: 0x%" PRIx64 "\n", dlopen_load);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    // Backup target process context
    if (!LoadContext(&ori_regs))
        return false;

    LOGI("riscv64: target process current sp: 0x%" PRIx64 "\n", ori_regs.sp);
    pt_regs call_regs;
    memcpy(&call_regs, &ori_regs, sizeof(pt_regs));

    call_regs.sp = ori_regs.sp - RoundUp(strlen(library) + 1, 8);
    RemoteCommand::Write(Pid(), call_regs.sp, (void *)library, strlen(library) + 1);

    call_regs.a0 = call_regs.sp;
    call_regs.a1 = RTLD_NOW;
    call_regs.ra = 0x0;
    call_regs.pc = dlopen_load;

    if (!StoreContext(&call_regs))
        return false;

    if (!Continue())
        return false;

    // Get return retval
    pt_regs retval_regs;
    LoadContext(&retval_regs);
    LOGI("riscv64: return 0x%" PRIx64 "\n", retval_regs.ra);

    // Restore origin context
    StoreContext(&ori_regs);
    return true;
}

} // namespace riscv64
