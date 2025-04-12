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
#include "command/core/cmd_write.h"
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
        {0,         0,                 0,  0 },
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
    LOGI("Usage: write|wd <ADDRESS> <OPTION>\n");
    LOGI("Option:\n");
    LOGI("    -s, --string <STRING>   overwrite string at dist address\n");
    LOGI("    -v, --value <VALUE>     overwrite value at dis address\n");
    ENTER();
    LOGI("core-parser> p 0x71edf4f8 -b\n");
    LOGI("Size: 0x28\n");
    LOGI("Object Name: java.lang.String\n");
    LOGI("    [0x10] virutal char[] values = \"AES128-GCM-SHA256\"\n");
    LOGI("    [0x0c] private int hash = -1058959256\n");
    LOGI("    [0x08] private final int count = 17\n");
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x04] private transient int shadow$_monitor_ = 536870912\n");
    LOGI("    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f817d58\n");
    LOGI("Binary:\n");
    LOGI("71edf4f8: 200000006f817d58  c0e1906800000022  X}.o....\"...h...\n");
    LOGI("71edf508: 472d383231534541  35324148532d4d43  AES128-GCM-SHA25\n");
    LOGI("71edf518: 0000000000000036  200000006f817d58  6.......X}.o....\n");
    ENTER();
    LOGI("core-parser> wd 71edf508 -s PenguinLetsGo\n");
    LOGI("New overlay [71bd5000, 71eea000)\n");
    LOGI("core-parser> p 0x71edf4f8 -b\n");
    LOGI("Size: 0x28\n");
    LOGI("Object Name: java.lang.String\n");
    LOGI("    [0x10] virutal char[] values = \"PenguinLetsGo\"\n");
    LOGI("    [0x0c] private int hash = -1058959256\n");
    LOGI("    [0x08] private final int count = 17\n");
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x04] private transient int shadow$_monitor_ = 536870912\n");
    LOGI("    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f817d58\n");
    LOGI("Binary:\n");
    LOGI("71edf4f8: 200000006f817d58  c0e1906800000022  X}.o....\"...h...\n");
    LOGI("71edf508: 4c6e6975676e6550  3532006f47737465  PenguinLetsGo.25\n");
    LOGI("71edf518: 0000000000000036  200000006f817d58  6.......X}.o....\n");
}
