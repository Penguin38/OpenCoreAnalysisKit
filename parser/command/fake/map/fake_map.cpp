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
#include "base/memory_map.h"
#include "zip/zip_file.h"
#include "common/bit.h"
#include "common/elf.h"
#include "command/fake/map/fake_map.h"
#include <unistd.h>
#include <getopt.h>
#include <memory>

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
        {"sysroot", required_argument,    0, 4},
    };

    int option = 0;
    char* sysroot_dirs = nullptr;
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
            case 4:
                option |= FAKE_SYSROOT;
                sysroot_dirs = optarg;
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
            uint64_t ld = 0;
            if (argc - optind > 2) ld = Utils::atol(argv[optind + 2]) & CoreApi::GetVabitsMask();
            return Append(addr, name, ld);
        }
    } else if (option & FAKE_SYSROOT) {
        FakeLinkMap::SysRoot(sysroot_dirs);
        return 0;
    }

    return 0;
}

int FakeLinkMap::AutoCreate() {
    if (CoreApi::Bits() == 64)
        return FakeLinkMap::AutoCreate64();
    else
        return FakeLinkMap::AutoCreate32();
}

int FakeLinkMap::Append(uint64_t addr, const char* name, uint64_t ld) {
    if (CoreApi::Bits() == 64)
        return FakeLinkMap::Append64(addr, name, ld);
    else
        return FakeLinkMap::Append32(addr, name, ld);
}

int FakeLinkMap::LD() {
    auto callback = [&](LinkMap* map) -> bool {
        FakeLinkMap::FakeLD(map);
        return false;
    };
    CoreApi::ForeachLinkMap(callback);
    return 0;
}

bool FakeLinkMap::FakeLD(LinkMap* map) {
    if (CoreApi::Bits() == 64)
        return FakeLinkMap::FakeLD64(map);
    else
        return FakeLinkMap::FakeLD32(map);
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
    LOGI("0x%" PRIx64 " %s\n", module_load, name);
    return module_load;
}

void FakeLinkMap::SysRoot(const char* path) {
    std::vector<char *> dirs;
    std::unique_ptr<char> newpath(strdup(path));
    char *token = strtok(newpath.get(), ":");
    while (token != nullptr) {
        dirs.push_back(token);
        token = strtok(nullptr, ":");
    }

    auto callback = [dirs](LinkMap* map) -> bool {
        if (map->l_ld())
            return false;

        std::unique_ptr<char> newname(strdup(map->name()));
        char *ori_file = strtok(newname.get(), "!");
        char *sub_file = strtok(NULL, "!");

        std::string filepath;
        for (char *dir : dirs) {
            if (Utils::SearchFile(dir, &filepath, ori_file))
                break;
        }
        if (!filepath.length())
            return false;

        LoadBlock* block = CoreApi::FindLoadBlock(map->l_addr(), false, false);
        if (!FakeLinkMap::DirectMmap(map, block, filepath.c_str(), sub_file))
            return false;

        if (!FakeLinkMap::FakeLD(map))
            block->removeMmap();

        return false;
    };
    CoreApi::ForeachLinkMap(callback);
}

bool FakeLinkMap::DirectMmap(LinkMap* handle, LoadBlock* block, const char* file, const char* subfile) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(file));
    if (subfile) {
        ZipFile zip;
        if (zip.open(file)) {
            LOGE("Zip open fail %s\n", file);
            return false;
        }

        ZipEntry* entry;
        if (subfile[0] == '/') {
            entry = zip.getEntryByName(subfile + 1);
        } else {
            entry = zip.getEntryByName(subfile);
        }
        if (!entry) {
            LOGE("%s Not found entry %s\n", file, subfile);
            return false;
        }

        if (!entry->IsUncompressed()) {
            LOGE("Not support compress zip %s!%s\n", file, entry->getFileName());
            return false;
        }

        std::unique_ptr<MemoryMap> submap(MemoryMap::MmapFile(file,
                                                              entry->getEntryTotalMemsz(),
                                                              entry->getFileOffset()));
        map = std::move(submap);
    }
    if (map) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
        if (!header->CheckLibrary(file))
            return false;

        if (block && block->vaddr() == handle->l_addr()) {
            block->setMmapFile(file, map->offset());
            return true;
        }
    }
    return false;
}

void FakeLinkMap::Usage() {
    LOGI("Usage: fake map [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    --ld                            calibrate link_map l_addr and l_ld\n");
    LOGI("    --auto                          auto create link_map\n");
    LOGI("    --append <ADDR> <NAME> [<LD>]   append link map\n");
    LOGI("    --sysroot <DIR:DIR>             sysroot and calibrate\n");
    ENTER();
    LOGI("core-parser> fake map --ld\n");
    LOGI("calibrate /apex/com.android.art/lib64/libart.so l_ld(7d5f20e8f8)\n");
    LOGI("calibrate /apex/com.android.art/lib64/libunwindstack.so l_ld(7d619fcb38)\n");
    LOGI("calibrate /apex/com.android.runtime/lib64/bionic/libc.so l_ld(7e0c7762e8)\n");
}
