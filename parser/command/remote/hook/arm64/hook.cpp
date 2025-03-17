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
#include "command/remote/hook/arm64/hook.h"
#include "command/remote/cmd_remote.h"
#include "command/remote/fakecore/process.h"
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>

namespace arm64 {

//follow android jni sign
static Hook::MethodTable kArm64MethodTable[] = {
    { "dlopen",   "AI",    2},
    { "dlclose",  "P",     1},
    { "mprotect", "PII",   3},
    { "malloc",   "I",     1},
    { "free",     "P",     1},
};

bool Hook::InjectLibrary(const char* library) {
    if (!CoreApi::IsRemote() || Env::CurrentRemotePid() != Pid()) {
        std::unique_ptr<FakeCore::Stream> process =
                std::make_unique<fakecore::Process>(Pid());
        std::unique_ptr<FakeCore> impl = FakeCore::Make(process);
        if (!impl)
            return false;

        impl->InitVaBits(39);
        impl->InitPageSize(sysconf(_SC_PAGE_SIZE));
        impl->InitRebuild(false);
        impl->execute(nullptr);
    }

    if (!library || !strlen(library))
        return false;

    LOGI("arm64: hook inject \"%s\"\n", library);

    uint64_t dlopen_load = CoreApi::DlSym("dlopen");
    if (!dlopen_load)
        return false;
    LOGI("arm64: hook found \"dlopen\" address: 0x%" PRIx64 "\n", dlopen_load);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    // Backup target process context
    if (!LoadContext(&ori_regs))
        return false;

    LOGI("arm64: target process current sp: 0x%" PRIx64 "\n", ori_regs.sp);
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

    LOGI("arm64: call dlopen(0x%" PRIx64 " \"%s\", 0x%" PRIx64 ")\n",
            call_regs.regs[0], library, call_regs.regs[1]);
    if (!Continue())
        return false;

    // Get return retval
    pt_regs retval_regs;
    LoadContext(&retval_regs);
    LOGI("arm64: return 0x%" PRIx64 "\n", retval_regs.regs[0]);

    // Restore origin context
    StoreContext(&ori_regs);
    return true;
}

bool Hook::CallMethod(const char* method, int argc, char* const argv[]) {
    if (!CoreApi::IsRemote() || Env::CurrentRemotePid() != Pid()) {
        std::unique_ptr<FakeCore::Stream> process =
                std::make_unique<fakecore::Process>(Pid());
        std::unique_ptr<FakeCore> impl = FakeCore::Make(process);
        if (!impl)
            return false;

        impl->InitVaBits(39);
        impl->InitPageSize(sysconf(_SC_PAGE_SIZE));
        impl->InitRebuild(false);
        impl->execute(nullptr);
    }

    uint64_t method_load = CoreApi::DlSym(method);
    if (!method_load)
        return false;
    LOGI("arm64: hook found \"%s\" address: 0x%" PRIx64 "\n", method, method_load);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    // Backup target process context
    if (!LoadContext(&ori_regs))
        return false;

    pt_regs call_regs;
    memcpy(&call_regs, &ori_regs, sizeof(pt_regs));

    // call method
    call_regs.regs[30] = 0x0;
    call_regs.pc       = method_load;
    if (!MethodContext(method, &call_regs, argc, argv))
        return false;

    if (!StoreContext(&call_regs))
        return false;

    if (!Continue())
        return false;

    // Get return retval
    pt_regs retval_regs;
    LoadContext(&retval_regs);
    LOGI("arm64: return 0x%" PRIx64 "\n", retval_regs.regs[0]);

    // Restore origin context
    StoreContext(&ori_regs);
    return true;
}

bool Hook::MethodContext(const char* method, pt_regs* call_regs,
                         int argc, char* const argv[]) {
    int count = sizeof(kArm64MethodTable)/sizeof(kArm64MethodTable[0]);
    for (int i = 0; i < count; ++i) {
        if (strcmp(method, kArm64MethodTable[i].method))
            continue;

        if (argc < kArm64MethodTable[i].min) {
            LOGE("method parameter number (%d < %d)\n", argc, kArm64MethodTable[i].min);
            return false;
        }

        uint32_t param_num = strlen(kArm64MethodTable[i].sign);
        LOGI("arm64: call method %s", method);
        for (int j = 0; j < param_num; ++j) {
            switch(kArm64MethodTable[i].sign[j]) {
                case 'Z':
                case 'B':
                case 'C':
                case 'S':
                case 'I':
                case 'J':
                case 'F':
                case 'D':
                case 'P':
                    if (argc > j)
                        call_regs->regs[j] = Utils::atol(argv[j]);
                    else
                        call_regs->regs[j] = 0x0;
                    break;
                case 'A':
                    if (argc > j) {
                        char* value = argv[j];
                        call_regs->sp = call_regs->sp - RoundUp(strlen(value) + 1, 0x10);
                        RemoteCommand::Write(Pid(), call_regs->sp, (void *)value, strlen(value) + 1);
                        call_regs->regs[j] = call_regs->sp;
                    } else
                        call_regs->regs[j] = 0x0;
                    break;
            }
            LOGI(" 0x%" PRIx64, call_regs->regs[j]);
        }
        ENTER();
        return true;
    }
    LOGE("Not support call method %s!!\n", method);
    return false;
}

} // namespace arm64
