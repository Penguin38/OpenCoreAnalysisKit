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
#include "command/cmd_write.h"
#include "base/utils.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>

int WriteCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || (argc < 3)) {
        usage();
        return 0;
    }

    uint64_t address = Utils::atol(argv[1]) & CoreApi::GetVabitsMask();
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"string",  required_argument, 0, 's'},
        {"value",   required_argument, 0, 'v'},
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "s:v:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                CoreApi::Write(address, optarg, strlen(optarg) + 1);
                break;
            case 'v':
                uint64_t value = Utils::atol(optarg);
                CoreApi::Write(address, value);
                break;
        }
    }
    return 0;
}

void WriteCommand::usage() {
    LOGI("Usage: write|wd <Address> <Option>\n");
    LOGI("Option:\n");
    LOGI("    --string|-s <STRING>:\n");
    LOGI("    --value|-v <VALUE>:\n");
}
