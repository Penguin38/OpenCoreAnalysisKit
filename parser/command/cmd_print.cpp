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
#include "runtime/mirror/object.h"
#include "command/cmd_print.h"
#include "base/utils.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>
#include <iomanip>

int PrintCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !argc)
        return 0;

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"--binary",  no_argument,       0,  'b'},
        {"--ref",     no_argument,       0,  'r'},
        {0,           0,                 0,   0 }
    };
    
    while ((opt = getopt_long(argc, argv, "e:f:012",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'b':
                break;
            case 'r':
                break;
        }
    }

    // reset
    optind = 0;

    // symbols init for later
    Android::Init();

    art::mirror::Object ref = Utils::atol(argv[0]);
    LOGI("0x%lx\n", ref.SizeOf());

    return 0;
}

void PrintCommand::usage() {
    LOGI("Usage: print|p object -[br]\n");
}

