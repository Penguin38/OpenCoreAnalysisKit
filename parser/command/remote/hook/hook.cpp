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
#include "command/remote/hook/hook.h"
#include "command/remote/hook/arm64/hook.h"
#include "command/remote/hook/arm/hook.h"
#include "command/remote/hook/x86_64/hook.h"
#include "command/remote/hook/x86/hook.h"
#include "command/remote/hook/riscv64/hook.h"
#include "command/remote/opencore/opencore.h"
#include <unistd.h>
#include <getopt.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <errno.h>
#include <linux/elf.h>
#include <sys/types.h>
#include <sys/wait.h>

int Hook::Main(int argc, char* const argv[]) {
    bool inject = false;
    char* library = nullptr;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        /* {"pid",     required_argument, 0, 'p'}, */
        {"inject",  no_argument,       0,  1 },
        {"lib",     required_argument, 0, 'l'},
    };

    while ((opt = getopt_long(argc, argv, "l:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 1:
                inject = true;
                break;
            case 'l':
                library = optarg;
                break;
        }
    }

    if (!CoreApi::IsRemote()) {
        LOGE("Only support core-parser on remote mode!\n");
        return 0;
    }

    int pid = Env::CurrentRemotePid();
    std::unique_ptr<Hook> impl = Hook::MakeArch(pid);
    if (impl && inject)
        impl->InjectLibrary(library);
    return 1;
}

std::unique_ptr<Hook> Hook::MakeArch(int pid) {
    std::unique_ptr<Hook> impl;
    std::string type = Opencore::DecodeMachine(pid);
    if (type == "arm64" || type == "ARM64") {
        impl = std::make_unique<arm64::Hook>(pid);
    } else if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::Hook>(pid);
    } else if (type == "x86_64" || type == "X86_64") {
        impl = std::make_unique<x86_64::Hook>(pid);
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::Hook>(pid);
    } else if (type == "riscv64" || type == "RISCV64") {
        impl = std::make_unique<riscv64::Hook>(pid);
    }
    return std::move(impl);
}

bool Hook::Continue() {
    if (ptrace(PTRACE_CONT, Pid(), NULL, 0) < 0)
        return false;

    int stat = 0;
    waitpid(Pid(), &stat, WUNTRACED);
    return true;
}

bool Hook::LoadContext(void *regs) {
    struct iovec ioVec;
    ioVec.iov_base = regs;
    ioVec.iov_len = RegsSize();

    if (ptrace(PTRACE_GETREGSET, Pid(), NT_PRSTATUS, &ioVec) < 0) {
        LOGI("%s %d: %s\n", __func__ , Pid(), strerror(errno));
        return false;
    }
    return true;
}

bool Hook::StoreContext(void *regs) {
    struct iovec ioVec;
    ioVec.iov_base = regs;
    ioVec.iov_len = RegsSize();

    if (ptrace(PTRACE_SETREGSET, Pid(), NT_PRSTATUS, &ioVec) < 0) {
        LOGI("%s %d: %s\n", __func__ , Pid(), strerror(errno));
        return false;
    }
    return true;
}

void Hook::Usage() {
    LOGI("Usage: remote hook [COMMAND] [OPTION]\n");
    LOGI("Command:\n");
    /* LOGI("    -p, --pid     target process pid\n"); */
    LOGI("    --inject      inject library\n");
    LOGI("    -l, --lib     set library path or name\n");
    ENTER();
    LOGI("core-parser> remote hook --inject -l libfdtrack.so\n");
    LOGI("x86_64: hook inject \"libfdtrack.so\"\n");
    LOGI("x86_64: hook found \"dlopen\" address: 0x7d9db3bc9b50\n");
    LOGI("x86_64: target process current rsp: 0x7fff424406e8\n");
    LOGI("x86_64: return 0xa7b93ee50bfa700d\n");
}
