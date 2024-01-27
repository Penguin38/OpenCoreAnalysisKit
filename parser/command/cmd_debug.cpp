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
#include "command/cmd_debug.h"
#include <unistd.h>
#include <getopt.h>
#include <string.h>

int DebugCommand::main(int argc, char* const argv[]) {
    if (argc < 2)
        return 0;

    if (strcmp(argv[0], "level")) {
        LOGI("Unkown debug cmd(%s)\n", argv[0]);
        return 0;
    }

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"debug",   no_argument,       0, Logger::LEVEL_DEBUG},
        {"info",    no_argument,       0, Logger::LEVEL_INFO},
        {"warn",    no_argument,       0, Logger::LEVEL_WARN},
        {"error",   no_argument,       0, Logger::LEVEL_ERROR},
        {"fatal",   no_argument,       0, Logger::LEVEL_FATAL},
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "01234",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                LOGI("Switch debug level %s\n", long_options[option_index].name);
                Logger::SetLevel(opt);
                break;
            default:
                LOGI("Unkown debug level.\n");
                break;
        }
    }

    // reset
    optind = 0;
    return 0;
}

void DebugCommand::usage() {
    LOGI("Usage: debug level --[debug|info|warn|error|fatal]\n");
}
