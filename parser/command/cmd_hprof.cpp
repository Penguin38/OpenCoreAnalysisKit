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
#include "android.h"
#include "command/cmd_hprof.h"
#include "runtime/hprof/hprof.h"
#include <unistd.h>
#include <getopt.h>

int HprofCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    if (!argc) {
        LOGE("ERROR: Please enter <FILE>\n");
        return 0;
    }

    int opt;
    bool visible = false;
    int option_index = 0;
    static struct option long_options[] = {
        {"visible",  no_argument,      0, 'v'},
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "v",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                visible = true;
                break;
        }
    }

    // reset
    optind = 0;

    art::hprof::DumpHeap(argv[0], visible);
    return 0;
}

void HprofCommand::usage() {
    LOGI("Usage: hprof <FILE> [option]\n");
    LOGI("       option:\n");
    LOGI("              --visible|-v\n");
}
