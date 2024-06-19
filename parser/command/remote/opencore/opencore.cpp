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
#include "command/remote/opencore/opencore.h"
#include "command/remote/opencore/x86_64/opencore.h"
#include "command/remote/opencore/x86/opencore.h"
#include "command/remote/opencore/arm64/opencore.h"
#include "command/remote/opencore/arm/opencore.h"
#include "command/remote/opencore/riscv64/opencore.h"
#include <unistd.h>
#include <getopt.h>
#include <sys/utsname.h>

int Opencore::Dump(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {"filter",  required_argument, 0, 'f'},
        {"dir",     required_argument, 0, 'd'},
        {"output",  required_argument, 0, 'o'},
        {"machine", required_argument, 0, 'm'},
    };

    while ((opt = getopt_long(argc, argv, "t:d:o:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                break;
            case 't':
                break;
            case 'd':
                break;
            case 'o':
                break;
            case 'm':
                break;
        }
    }
    return 0;
}

void Opencore::Usage() {
    LOGI("Usage: remote core -p <PID> [Option]...\n");
    LOGI("Option:\n");
    LOGI("   --pid|-p <PID>\n");
    LOGI("   --dir|-d <DIR>\n");
    LOGI("   --machine|-m <Machine>\n");
    LOGI("Machine:\n");
    LOGI("     { arm64, arm, x86_64, x86, riscv64 }\n");
    LOGI("   --output|-o <COREFILE>\n");
    LOGI("   --filter|-f <Filter>\n");
    LOGI("Filter:\n");
    LOGI("     0x01: filter-special-vma\n");
    LOGI("     0x02: filter-file-vma\n");
    LOGI("     0x04: filter-shared-vma\n");
    LOGI("     0x08: filter-sanitizer-shadow-vma\n");
    LOGI("     0x10: filter-non-read-vma\n");
}
