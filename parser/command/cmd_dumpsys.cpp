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
#include "android.h"
#include "command/cmd_dumpsys.h"
#include "com/android/server/am/ActivityManagerService.h"
#include <string>

typedef int (*DumpsysCall)(int argc, char* const argv[]);
struct DumpsysOption {
    const char* cmd;
    DumpsysCall call;
};

static DumpsysOption dumpsys_option[] = {
    { "activity", com::android::server::am::ActivityManagerService::Main },
};

int DumpsysCommand::main(int argc, char* const argv[]) {
    if (!(argc > 1)) {
        usage();
        return 0;
    }

    int count = sizeof(dumpsys_option)/sizeof(dumpsys_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], dumpsys_option[index].cmd)) {
            return dumpsys_option[index].call(argc - 1, &argv[1]);
        }
    }
    return 0;
}

void DumpsysCommand::usage() {
    LOGI("Usage: dumpsys <SERVICE> [Option]...\n");
}
