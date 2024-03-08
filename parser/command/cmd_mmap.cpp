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
#include "base/utils.h"
#include "command/cmd_mmap.h"
#include "common/exception.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>

int MmapCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || argc < 3)
        return 0;

    int opt;
    uint64_t begin = Utils::atol(argv[1]) & CoreApi::GetVabitsMask();
    char* file;
    uint64_t offset = 0x0;
    int remove = 0;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"file",    required_argument,  0,  'f'},
        {"offset",  required_argument,  0,  'o'},
        {"remove-mmap",  no_argument,   0,   1 },
        {"remove-overlay", no_argument, 0,   2 },
        {0,         0,                  0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "f:o:012",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'f':
                file = optarg;
                break;
            case 'o':
                offset = Utils::atol(optarg);
                break;
            case 1:
                remove |= REMOVE_MMAP;
                break;
            case 2:
                remove |= REMOVE_OVERLAY;
                break;
        }
    }

    LoadBlock* block = CoreApi::FindLoadBlock(begin, false);
    if (!block)
        throw InvalidAddressException(begin);

    if (!remove) {
        block->setMmapFile(file, offset);
    } else {
        if (remove & REMOVE_MMAP)
            block->removeMmap();

        if (remove & REMOVE_OVERLAY)
            block->removeOverlay();
    }

    return 0;
}

void MmapCommand::usage() {
    LOGI("Usage: mmap <ADDR> <OPT>\n");
    LOGI("       option:\n");
    LOGI("         [--file|-f] <FILE>\n");
    LOGI("         [--offset|-o] <OFFSET>\n");
    LOGI("         --remove-mmap\n");
    LOGI("         --remove-overlay\n");
}
