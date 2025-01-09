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
#include "android.h"
#include "base/utils.h"
#include "command/env.h"
#include "command/fake/stack/fake_java_stack.h"
#include "runtime/thread_list.h"
#include <unistd.h>
#include <getopt.h>

int FakeJavaStack::OptionJavaStack(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pc",    required_argument, 0,  0 },
        {"sp",    required_argument, 0,  1 },
        {"clean", no_argument,       0, 'c'},
    };

    uint64_t pc = 0x0;
    uint64_t sp = 0x0;
    bool clean_fake = false;
    while ((opt = getopt_long(argc, argv, "0:1:c",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                pc = Utils::atol(optarg) & CoreApi::GetVabitsMask();
                break;
            case 1:
                sp = Utils::atol(optarg) & CoreApi::GetVabitsMask();
                break;
            case 'c':
                clean_fake = true;
                break;
        }
    }

    art::Thread* current = nullptr;
    int pid = Env::CurrentPid();
    if (Android::IsSdkReady() && art::Runtime::Current().Ptr()) {
        current = art::Runtime::Current().GetThreadList().FindThreadByTid(pid);
    }

    if (!current)
        return 0;

    if (clean_fake) {
        current->GetFakeFrame().FillFake(0x0, 0x0);
    } else {
        if (strcmp(current->GetStateDescriptor(), "Runnable"))
            return 0;

        if (pc != 0x0 && sp != 0x0) {
            current->GetFakeFrame().FillFake(pc, sp);
        } else {
            // todo auto analysis
        }
    }
    return 0;
}

void FakeJavaStack::Usage() {
    LOGI("Usage: fake stack --pc <PC> --sp <SP> [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -c, --clean    clean fake java stack pc, sp\n");
}
