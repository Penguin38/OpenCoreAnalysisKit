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
        {"read",    no_argument,       0,  'r' },
        {"write",   no_argument,       0,  'w' },
        {"exec",    no_argument,       0,  'x' },
        {"none",    no_argument,       0,  'n' },
        {0,         0,                 0,   0  },
    };

    uint64_t vaddr = 0x0;
    uint64_t size = 0x0;
    int flags = 0x0;
    int def_flags = Block::FLAG_R | Block::FLAG_W;
    while ((opt = getopt_long(argc, argv, "v:s:rwxn",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'v':
                vaddr = Utils::atol(optarg);
                break;
            case 's':
                size = Utils::atol(optarg);
                break;
            case 'r':
                flags |= Block::FLAG_R;
                break;
            case 'w':
                flags |= Block::FLAG_W;
                break;
            case 'x':
                flags |= Block::FLAG_X;
                break;
            case 'n':
                def_flags = 0;
                break;
        }
    }

    if (!flags) flags = def_flags;
    CoreApi::NewLoadBlock(vaddr, size, flags);
    return 0;
}

void FakeLoadBlock::Usage() {
    LOGI("Usage: fake load <OPTION>\n");
    LOGI("Option:\n");
    LOGI("    -v, --vaddr      if vaddr is NULL, then parser chooses address\n");
    LOGI("    -s, --size       set load block size\n");
    LOGI("    -r, --read       set load block readable flags (default)\n");
    LOGI("    -w, --write      set load block writable flags (default)\n");
    LOGI("    -x, --exec       set load block executable flags\n");
    LOGI("    -n, --none       set load block none flags\n");
    ENTER();
    LOGI("core-parser> fake load --vaddr 7fd10b0000 --size 0x4000\n");
    LOGI("New overlay [7fd10b0000, 7fd10b4000)\n");
}
