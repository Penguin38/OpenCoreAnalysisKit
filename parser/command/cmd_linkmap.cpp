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
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>

int LinkMapCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    dump_ori = false;
    num = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"origin",  no_argument,       0,  'o'},
        {"sym",     required_argument, 0,  's'},
        {0,           0,               0,   0 }
    };

    while ((opt = getopt_long(argc, argv, "os:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                dump_ori = true;
                break;
            case 's':
                num = std::atoi(optarg);
                break;
        }
    }

    if (!num) LOGI("NUM LINKMAP       REGION                   FLAGS NAME\n");
    int pos = 0;
    auto callback = [&](LinkMap* map) -> bool {
        pos++;
        if (!num) {
            ShowLinkMap(pos, map);
        } else {
            if (num == pos) {
                ShowLinkMapSymbols(map);
                return true;
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
        std::string valid;
        if (block->isValid()) {
            valid.append("[*]");
            if (block->isOverlayBlock()) {
                valid.append("(OVERLAY)");
            } else if (block->isMmapBlock()) {
                valid.append("(MMAP");
                if (block->GetMmapOffset()) {
                    valid.append(" ");
                    valid.append(Utils::ToHex(block->GetMmapOffset()));
                }
                valid.append(")");
            }
        } else {
            valid.append("[EMPTY]");
        }

        LOGI("%3d 0x%lx  [%lx, %lx)  %s  %s %s\n", pos, map->map(), block->vaddr(), block->vaddr() + block->size(),
                block->convertFlags().c_str(), name.c_str(), valid.c_str());
    } else {
        LOGI("%3d 0x%lx  [%lx, %lx)  ---  %s [unknown]\n", pos, map->map(), map->begin(), map->begin(), map->name());
    }
}

void LinkMapCommand::ShowLinkMapSymbols(LinkMap* map) {
    LOGI("VALUE             INFO              NAME\n");
    for (const auto& entry : map->GetCurrentSymbols()) {
        LOGI("%016lx  %016lx  %s\n", entry.offset, entry.type, entry.symbol.c_str());
    }
}

void LinkMapCommand::usage() {
    LOGI("Usage: map [option..]\n");
    LOGI("Option:\n");
    LOGI("  --ori|-o: show origin link map.\n");
    LOGI("  --sym|-s <NUM>: show link map current symbols.\n");
}

