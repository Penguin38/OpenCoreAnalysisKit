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
#include "command/cmd_env.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>

int EnvCommand::main(int argc, char* const argv[]) {
    if (!argc)
        return dumpEnv();

    if (strcmp(argv[0], "config")) {
        LOGI("unknown command (%s)\n", argv[0]);
        return 0;
    }

    int opt;
    int option_index = 0;
    int current_sdk = Android::UPSIDE_DOWN_CAKE;
    static struct option long_options[] = {
        {"sdk",     required_argument, 0,  0 },
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "0:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                current_sdk = atoi(optarg);
                Android::OnSdkChanged(current_sdk);
                break;
        }
    }

    // reset
    optind = 0;
    return 0;
}

int EnvCommand::dumpEnv() {
    if (CoreApi::IsReady()) {
        CoreApi::Dump();
    }

    if (Android::IsReady()) {
        Android::Dump();
    }

    return 0;
}

void EnvCommand::usage() {
    LOGI("Usage: env [config] --[opt] [value]\n");
}
