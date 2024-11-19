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
#include "command/fake/core/fake_core.h"
#include "command/fake/core/lp64/restore.h"
#include "command/fake/core/lp32/restore.h"
#include "tombstone/tombstone.h"
#include <unistd.h>
#include <getopt.h>

int FakeCore::OptionCore(int argc, char* const argv[]) {

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"tomb",        required_argument, 0, 't'},
        {"rebuild",     no_argument,       0, 'r'},
        {"output",      required_argument, 0, 'o'},
    };

    bool tomb = false;
    char* tomb_file = nullptr;
    bool restore = false;
    char* output = nullptr;

    while ((opt = getopt_long(argc, argv, "t:ro:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                tomb = true;
                restore = false;
                tomb_file = optarg;
                break;
            case 'r':
                tomb = false;
                restore = true;
                break;
            case 'o':
                output = optarg;
                break;
        }
    }

    if (restore) {
        std::string filename;
        if (!output) {
            if (!CoreApi::IsRemote()) {
                filename = CoreApi::GetName();
                filename.append(".fakecore");
            }
        } else {
            filename = output;
        }
        if (CoreApi::Bits() == 64) {
            lp64::Restore::execute(filename.c_str());
        } else {
            lp32::Restore::execute(filename.c_str());
        }
    } else if (tomb) {
        android::Tombstone tombstone(tomb_file);
        // do nothing
    }
    return 0;
}

void FakeCore::Usage() {
    LOGI("Usage: fake core <OPTION...>\n");
    LOGI("Option:\n");
    LOGI("    -t, --tomb <TOMBSTONE>    build tombstone fakecore\n");
    LOGI("    -r, --rebuild             rebuild current environment core.\n");
    LOGI("    -o, --output <COREFILE>   set current fakecore path\n");
    ENTER();
    LOGI("core-parser> fake core -r\n");
    LOGI("FakeCore: saved [core.opencore.tester_6118_Thread-2_6146_1720691326.fakecore]\n");
}
