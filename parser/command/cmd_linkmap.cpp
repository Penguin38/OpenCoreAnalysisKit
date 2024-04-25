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
#include "command/cmd_linkmap.h"
#include "api/core.h"

int LinkMapCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    auto callback = [](LinkMap* map) -> bool {
        LoadBlock* block = map->block();
        if (block) {
            std::string name;
            if (block->isMmapBlock()) {
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
                    valid.append("(MMAP)");
                }
            } else {
                valid.append("[EMPTY]");
            }

            LOGI("0x%lx  [%lx, %lx)  %s %s\n", map->map(), block->vaddr(), block->vaddr() + block->size(),
                                       name.c_str(), valid.c_str());
        } else {
            LOGI("0x%lx  [%lx, %lx)  %s [unknown]\n", map->map(), block->vaddr(), block->vaddr() + block->size(),
                                       block->name().c_str());
        }
        return false;
    };
    LOGI("LINKMAP       REGION                    NAME\n");
    CoreApi::ForeachLinkMap(callback);

    return 0;
}

