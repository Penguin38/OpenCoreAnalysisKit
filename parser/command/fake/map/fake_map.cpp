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
#include "command/fake/map/fake_map.h"
#include <unistd.h>
#include <getopt.h>

int FakeLinkMap::OptionMap(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"ld",         no_argument,       0, 1},
        {"auto",       no_argument,       0, 2},
        {"append",     no_argument,       0, 3},
    };

    int option = 0;
    while ((opt = getopt_long(argc, argv, "a",
                long_options, &option_index)) != -1) {
        switch(opt) {
            case 1:
                option |= FAKE_LD;
                break;
            case 2:
                option |= FAKE_AUTO_CREATE;
                break;
            case 3:
                option |= FAKE_APPEND;
                break;
        }
    }

    if (option & FAKE_LD)
        return LD();
    else if (option & FAKE_AUTO_CREATE)
        return AutoCreate();
    else if (option & FAKE_APPEND) {
        if (argc - optind > 1) {
            uint64_t addr = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
            char* name = argv[optind + 1];
            return Append(addr, name);
        }
    }

    return 0;
}

int FakeLinkMap::AutoCreate() {
    if (CoreApi::Bits() == 64)
        return FakeLinkMap::AutoCreate64();
    else
        return FakeLinkMap::AutoCreate32();
}

int FakeLinkMap::Append(uint64_t addr, const char* name) {
    if (CoreApi::Bits() == 64)
        return FakeLinkMap::Append64(addr, name);
    else
        return FakeLinkMap::Append32(addr, name);
}

int FakeLinkMap::LD() {
    auto callback = [&](LinkMap* map) -> bool {
        if (CoreApi::Bits() == 64)
            FakeLinkMap::FakeLD64(map);
        else
            FakeLinkMap::FakeLD32(map);
        return false;
    };
    CoreApi::ForeachLinkMap(callback);
    return 0;
}

uint64_t FakeLinkMap::FindModuleFromLoadBlock(const char* name) {
    if (!name) return 0;

    uint64_t module_load = 0x0;
    std::vector<LoadBlock *> tmps;

    auto callback = [&](LoadBlock *block) -> bool {
        if (block->filename() != name)
            return false;

        if (block->flags() & Block::FLAG_X) {
            if (tmps.size() == 0) {
                module_load = block->vaddr();
                return true;
            }

            for (const auto& link : tmps) {
                uint64_t cloc_vaddr = block->vaddr() - block->pageoffset() + link->pageoffset();
                if (link->vaddr() > cloc_vaddr)
                    continue;

                if (link->vaddr() <= cloc_vaddr)
                    module_load = link->vaddr();

                if (link->vaddr() == cloc_vaddr)
                    return true;
            }
        } else {
            tmps.push_back(block);
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, false, false);
    LOGI("0x%lx %s\n", module_load, name);
    return module_load;
}

void FakeLinkMap::Usage() {
    LOGI("Usage: fake map [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    --ld                      calibrate link_map l_addr and l_ld\n");
    LOGI("    --auto                    auto create link_map\n");
    LOGI("    --append <ADDR> <NAME>    append link map\n");
    ENTER();
    LOGI("core-parser> fake map --ld\n");
    LOGI("calibrate /apex/com.android.art/lib64/libart.so l_ld(7d5f20e8f8)\n");
    LOGI("calibrate /apex/com.android.art/lib64/libunwindstack.so l_ld(7d619fcb38)\n");
    LOGI("calibrate /apex/com.android.runtime/lib64/bionic/libc.so l_ld(7e0c7762e8)\n");
}
