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
#include "command/cmd_linkmap.h"
#include "common/elf.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>

int LinkMapCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    dump_ori = false;
    dump_all = false;
    num = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"origin",  no_argument,       0,  'o'},
        {"sym",     required_argument, 0,  's'},
        {"all",     no_argument,       0,  'a'},
    };

    while ((opt = getopt_long(argc, argv, "aos:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                dump_all = true;
                break;
            case 'o':
                dump_ori = true;
                break;
            case 's':
                num = std::atoi(optarg);
                break;
        }
    }

    if (!num) LOGI(ANSI_COLOR_LIGHTRED "NUM LINKMAP       REGION                   FLAGS NAME\n" ANSI_COLOR_RESET);
    int pos = 0;
    auto callback = [&](LinkMap* map) -> bool {
        pos++;
        if (!num && !dump_all) {
            ShowLinkMap(pos, map);
        } else {
            if (num == pos || dump_all) {
                LOGI(ANSI_COLOR_LIGHTRED "VADDR             SIZE              INFO              NAME\n" ANSI_COLOR_RESET);
                if (dump_all)
                    LOGI("LIB: " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, map->name());
                ShowLinkMapSymbols(map);
                return true && !dump_all;
            }
        }
        return false;
    };
    CoreApi::ForeachLinkMap(callback);
    return 0;
}

void LinkMapCommand::ShowLinkMap(int pos, LinkMap* map) {
    LoadBlock* block = map->block();
    if (block) {
        std::string name;
        if (!dump_ori && block->isMmapBlock()) {
            name = block->name();
        } else {
            name = map->name();
        }
        LOGI("%3d " ANSI_COLOR_YELLOW "0x%lx" ANSI_COLOR_CYAN "  [%lx, %lx)" ANSI_COLOR_RESET "  %s  " ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET " %s\n",
                pos, map->map(), block->vaddr(), block->vaddr() + block->size(),
                block->convertFlags().c_str(), name.c_str(), block->convertValids().c_str());
    } else {
        LOGW("%3d 0x%lx  [%lx, ----)  ---  %s [unknown]\n", pos, map->map(), map->begin(), map->name());
    }
}

void LinkMapCommand::ShowLinkMapSymbols(LinkMap* map) {
    for (const auto& entry : map->GetCurrentSymbols()) {
        uint64_t offset = entry.offset;
        if (CoreApi::GetMachine() == EM_ARM)
            offset &= (CoreApi::GetPointMask() - 1);
        LOGI(ANSI_COLOR_CYAN "%016lx" ANSI_COLOR_RESET "  %016lx  %016lx  " ANSI_COLOR_YELLOW "%s\n" ANSI_COLOR_RESET,
                map->l_addr() + offset, entry.size, entry.type, entry.symbol.c_str());
    }
}

void LinkMapCommand::usage() {
    LOGI("Usage: map [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -o, --ori         show origin link map\n");
    LOGI("    -s, --sym <NUM>   show link map current symbols\n");
    LOGI("    -a, --all         show all link map current symbols\n");
    ENTER();
    LOGI("core-parser> map\n");
    LOGI("NUM LINKMAP       REGION                   FLAGS NAME\n");
    LOGI("  1 0x791af2b6d0e0  [5a224127f000, 5a2241282000)  r--  /system/bin/app_process64 [*]\n");
    LOGI("  2 0x791af2dd90e0  [791af2cbd000, 791af2cfd000)  r--  /system/bin/linker64 [*]\n");
    LOGI("  3 0x791af2b6d330  [7ffc73ae7000, 7ffc73ae8000)  r-x  [vdso] [*]\n");
    LOGI("  4 0x791af2b6d580  [791af0e08000, 791af0eb3000)  r--  /system/lib64/libandroid_runtime.so [*]\n");
    LOGI("  ...\n");
    ENTER();
    LOGI("core-parser> map --sym 3\n");
    LOGI("VADDR             SIZE              INFO              NAME\n");
    LOGI("00007ffc73ae7a10  000000000000002a  0000000000000022  getcpu\n");
    LOGI("00007ffc73ae77c0  00000000000001de  0000000000000022  clock_gettime\n");
    LOGI("00007ffc73ae77a0  0000000000000015  0000000000000012  __vdso_time\n");
    LOGI("00007ffc73ae7610  000000000000018a  0000000000000022  gettimeofday\n");
    LOGI("00007ffc73ae7a10  000000000000002a  0000000000000012  __vdso_getcpu\n");
    LOGI("00007ffc73ae79a0  0000000000000047  0000000000000022  clock_getres\n");
    LOGI("00007ffc73ae77a0  0000000000000015  0000000000000022  time\n");
    LOGI("00007ffc73ae79a0  0000000000000047  0000000000000012  __vdso_clock_getres\n");
    LOGI("00007ffc73ae77c0  00000000000001de  0000000000000012  __vdso_clock_gettime\n");
    LOGI("00007ffc73ae7610  000000000000018a  0000000000000012  __vdso_gettimeofday\n");
}

