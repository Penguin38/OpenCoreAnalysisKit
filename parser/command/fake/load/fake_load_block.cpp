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
#include "base/utils.h"
#include "command/fake/load/fake_load_block.h"
#include <unistd.h>
#include <getopt.h>

int FakeLoadBlock::OptionLoad(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"vaddr",   required_argument, 0,  'v' },
        {"size",    required_argument, 0,  's' },
    };

    uint64_t vaddr = 0x0;
    uint64_t size = 0x0;
    while ((opt = getopt_long(argc, argv, "v:s:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                vaddr = Utils::atol(optarg);
                break;
            case 's':
                size = Utils::atol(optarg);
                break;
        }
    }
    CoreApi::NewLoadBlock(vaddr, size);
    return 0;
}

void FakeLoadBlock::Usage() {
    LOGI("Usage: fake load <OPTION>\n");
    LOGI("Option:\n");
    LOGI("    -v, --vaddr      if vaddr is NULL, then parser chooses address\n");
    LOGI("    -s, --size       set load block size\n");
    ENTER();
    LOGI("core-parser> fake load --vaddr 7fd10b0000 --size 0x4000\n");
    LOGI("New overlay [7fd10b0000, 7fd10b4000)\n");
}
