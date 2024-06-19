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
#include "command/remote/cmd_remote.h"
#include "command/remote/opencore/opencore.h"
#include <unistd.h>
#include <getopt.h>

typedef int (*RemoteCall)(int argc, char* const argv[]);
struct RemoteOption {
    const char* cmd;
    RemoteCall call;
};

RemoteOption remote_option[] = {
    { "core", Opencore::Dump },
};

int RemoteCommand::main(int argc, char* const argv[]) {
    if (!(argc > 1)) {
        usage();
        return 0;
    }

    int count = sizeof(remote_option)/sizeof(remote_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], remote_option[index].cmd)) {
            return remote_option[index].call(argc - 1, &argv[1]);
        }
    }
    LOGI("unknown command (%s)\n", argv[1]);
    return 0;
}

void RemoteCommand::usage() {
    LOGI("Usage: remote <COMMAND> [option] ...\n");
    LOGI("Command:\n");
    LOGI("    core\n");
    LOGI("\n");
    Opencore::Usage();
}
