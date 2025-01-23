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
#include "command/android/cmd_hprof.h"
#include "runtime/hprof/hprof.h"
#include <unistd.h>
#include <getopt.h>

int HprofCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    bool visible = false;
    bool quick = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"visible",  no_argument,      0, 'v'},
        {"quick",    no_argument,      0, 'q'},
        {0,          0,                0,  0 },
    };

    while ((opt = getopt_long(argc, argv, "vq",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                visible = true;
                break;
            case 'q':
                quick = true;
                break;
        }
    }

    std::string filename;
    if (!(optind < argc)) {
        filename = CoreApi::GetName();
        filename.append(".hprof");
    } else {
        filename = argv[optind];
    }

    art::hprof::DumpHeap(filename.c_str(), visible, quick);
    return 0;
}

void HprofCommand::usage() {
    LOGI("Usage: hprof [<FILE>] [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -v, --visible     show hprof detail\n");
    LOGI("    -q, --quick       fast dump hprof\n");
    ENTER();
    LOGI("core-parser> hprof /tmp/1.hprof\n");
    LOGI("hprof: heap dump /tmp/1.hprof starting...\n");
    LOGI("hprof: heap dump completed, scan objects (306330).\n");
    LOGI("hprof: saved [/tmp/1.hprof].\n");
}
