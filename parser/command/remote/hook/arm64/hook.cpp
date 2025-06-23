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
#include <sys/mman.h>
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
    { "mmap",     "PIIIII",5},
    { "munmap",   "PI",    2},
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
        impl->InitMask(FakeCore::NO_FAKE_REBUILD);
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
        impl->InitMask(FakeCore::NO_FAKE_REBUILD);
        impl->execute(nullptr);
    }

    bool no_error;
    CallMethodInner(method, argc, argv, &no_error);
    return no_error;
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

uint64_t Hook::CallMethodInner(const char* method, int argc, char* const argv[], bool* no_error) {
    uint64_t method_load = CoreApi::DlSym(method);
    if (!method_load) {
        *no_error = false;
        return -1;
    }
    LOGI("arm64: hook found \"%s\" address: 0x%" PRIx64 "\n", method, method_load);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    // Backup target process context
    if (!LoadContext(&ori_regs)) {
        *no_error = false;
        return -1;
    }

    pt_regs call_regs;
    memcpy(&call_regs, &ori_regs, sizeof(pt_regs));

    // call method
    call_regs.regs[30] = 0x0;
    call_regs.pc       = method_load;
    if (!MethodContext(method, &call_regs, argc, argv)
            || !StoreContext(&call_regs)
            || !Continue()) {
        *no_error = false;
        return -1;
    }

    // Get return retval
    pt_regs retval_regs;
    LoadContext(&retval_regs);
    LOGI("arm64: return 0x%" PRIx64 "\n", retval_regs.regs[0]);

    // Restore origin context
    StoreContext(&ori_regs);
    *no_error = true;
    return retval_regs.regs[0];
}

uint32_t kSaveContext[] = {
    0xd10403ff, // sub sp, sp, #0x100
    0xa9007bfd, // stp x29, x30, [sp]
    0xa9016ffc, // stp x28, x27, [sp, #16]
    0xa90267fa, // stp x26, x25, [sp, #32]
    0xa9035ff8, // stp x24, x23, [sp, #48]
    0xa90457f6, // stp x22, x21, [sp, #64]
    0xa9054ff4, // stp x20, x19, [sp, #80]
    0xa90647f2, // stp x18, x17, [sp, #96]
    0xa9073ff0, // stp x16, x15, [sp, #112]
    0xa90837ee, // stp x14, x13, [sp, #128]
    0xa9092fec, // stp x12, x11, [sp, #144]
    0xa90a27ea, // stp x10, x9, [sp, #160]
    0xa90b1fe8, // stp x8, x7, [sp, #176]
    0xa90c17e6, // stp x6, x5, [sp, #192]
    0xa90d0fe4, // stp x4, x3, [sp, #208]
    0xa90e07e2, // stp x2, x1, [sp, #224]
    0xf9007be0, // str x0, [sp, #240]
    0x910003fd, // mov x29, sp
};

uint32_t kRestoreContext[] = {
    0xf9407be0, // ldr x0, [sp, #240]
    0xa94e07e2, // ldp x2, x1, [sp, #224]
    0xa94d0fe4, // ldp x4, x3, [sp, #208]
    0xa94c17e6, // ldp x6, x5, [sp, #192]
    0xa94b1fe8, // ldp x8, x7, [sp, #176]
    0xa94a27ea, // ldp x10, x9, [sp, #160]
    0xa9492fec, // ldp x12, x11, [sp, #144]
    0xa94837ee, // ldp x14, x13, [sp, #128]
    0xa9473ff0, // ldp x16, x15, [sp, #112]
    0xa94647f2, // ldp x18, x17, [sp, #96]
    0xa9454ff4, // ldp x20, x19, [sp, #80]
    0xa94457f6, // ldp x22, x21, [sp, #64]
    0xa9435ff8, // ldp x24, x23, [sp, #48]
    0xa94267fa, // ldp x26, x25, [sp, #32]
    0xa9416ffc, // ldp x28, x27, [sp, #16]
    0xa9407bfd, // ldp x29, x30, [sp]
    0x910403ff, // add sp, sp, 0x100
};

uint32_t kReturn[] = {
    0xd65f03c0, // ret
};

uint32_t kTmpBlrMethod[] = {
    0xd280001e, // mov x30, #0x0
    0xf2a0001e, // movk x30, #0x0, lsl #16
    0xf2c0001e, // movk x30, #0x0, lsl #32
    // 0xf2e0001e, // movk x30, #0x0, lsl #48
    0xd63f03c0, // blr x30
};

static void CreateBlrCode(uint32_t* code, uint64_t addr) {
    uint32_t inst1 = addr & 0xFFFF;
    uint32_t inst2 = (addr >> 16) & 0xFFFF;
    uint32_t inst3 = (addr >> 32) & 0xFFFF;
    // uint32_t inst4 = (addr >> 48) & 0xFFFF;

    memcpy(code, kTmpBlrMethod, sizeof(kTmpBlrMethod));
    code[0] |= (inst1 << 5);
    code[1] |= (inst2 << 5);
    code[2] |= (inst3 << 5);
    // code[3] |= (inst4 << 5);
}

bool Hook::InlineMethod(int argc, char* const argv[]) {
    if (!CoreApi::IsRemote() || Env::CurrentRemotePid() != Pid()) {
        std::unique_ptr<FakeCore::Stream> process =
                std::make_unique<fakecore::Process>(Pid());
        std::unique_ptr<FakeCore> impl = FakeCore::Make(process);
        if (!impl)
            return false;

        impl->InitVaBits(39);
        impl->InitPageSize(sysconf(_SC_PAGE_SIZE));
        impl->InitMask(FakeCore::NO_FAKE_REBUILD);
        impl->execute(nullptr);
    }

    if (argc < 2)
        return false;

    uint64_t inline_addr = Utils::atol(argv[0]);
    uint64_t target_addr = Utils::atol(argv[1]);
    LOGI("arm64: hook inline: %lx, target: %lx\n", inline_addr, target_addr);

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(Pid());

    uint32_t ori_inst[4] = { 0x0, 0x0, 0x0, 0x0 };
    if (!RemoteCommand::Read(Pid(), inline_addr, sizeof(ori_inst), (uint8_t*)ori_inst)) {
        LOGE("arm64: inline inst read fail!\n");
        return false;
    }

    uint64_t test_target;
    if (!RemoteCommand::Read(Pid(), target_addr, sizeof(test_target), (uint8_t*)&test_target)) {
        LOGE("arm64: test target addr fail!\n");
        return false;
    }

    bool no_error;
    std::string zero = Utils::ToHex(0x0);
    std::string fd = Utils::ToHex(-1ULL);
    std::string size = Utils::ToHex(sysconf(_SC_PAGE_SIZE));
    std::string prot = Utils::ToHex(PROT_READ | PROT_WRITE);
    std::string flags = Utils::ToHex(MAP_PRIVATE | MAP_ANON);
    int mmap_argc = 6;
    char* mmap_argv[6] = {
        const_cast<char*>(zero.c_str()),
        const_cast<char*>(size.c_str()),
        const_cast<char*>(prot.c_str()),
        const_cast<char*>(flags.c_str()),
        const_cast<char*>(fd.c_str()),
        const_cast<char*>(zero.c_str())
    };
    uint64_t mem = CallMethodInner("mmap", mmap_argc, mmap_argv, &no_error);
    if ((void *)mem == MAP_FAILED || !no_error)
        return false;

    // write machine code
    RemoteCommand::Write(Pid(), mem, (void *)kSaveContext, sizeof(kSaveContext));

    // target hook method
    uint32_t blr_code[4];
    CreateBlrCode(blr_code, target_addr);
    RemoteCommand::Write(Pid(), mem + sizeof(kSaveContext),
                        (void *)blr_code, sizeof(blr_code));

    // return inline method
    RemoteCommand::Write(Pid(), mem + sizeof(kSaveContext) + sizeof(blr_code),
                        (void *)kRestoreContext, sizeof(kRestoreContext));

    RemoteCommand::Write(Pid(), mem + sizeof(kSaveContext) + sizeof(blr_code) + sizeof(kRestoreContext),
                        (void *)&ori_inst, sizeof(ori_inst));
    RemoteCommand::Write(Pid(), mem + sizeof(kSaveContext) + sizeof(blr_code) + sizeof(kRestoreContext) + sizeof(ori_inst),
                        (void *)kReturn, sizeof(kReturn));

    std::string addr = Utils::ToHex(mem);
    prot = Utils::ToHex(PROT_READ | PROT_EXEC);
    int mprotect_argc = 3;
    char* mprotect_argv[3] = {
        const_cast<char*>(addr.c_str()),
        const_cast<char*>(size.c_str()),
        const_cast<char*>(prot.c_str()),
    };
    if (CallMethodInner("mprotect", mprotect_argc, mprotect_argv, &no_error)
            || !no_error)
        return false;

    CreateBlrCode(blr_code, mem);
    RemoteCommand::Write(Pid(), inline_addr, (void *)blr_code, sizeof(blr_code));

    return true;
}

} // namespace arm64
