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
#include "common/load_block.h"

uint64_t LoadBlock::begin(int opt) {
    if (mOverlay && (opt & OPT_READ_OVERLAY))
        return mOverlay->data();
    if (mMmap && (opt & OPT_READ_MMAP))
        return mMmap->data();
    if (oraddr() && (opt & OPT_READ_OR))
        return oraddr();
    return 0x0;
}

bool LoadBlock::isValid() {
    if ((flags() & FLAG_R) && (isValidBlock() || mMmap || mOverlay))
        return true;
    return false;
}

bool LoadBlock::virtualContains(uint64_t addr) {
    uint64_t clocaddr = addr & mVabitsMask;
    if (clocaddr >= vaddr() && clocaddr < (vaddr() + size()))
        return true;
    return false;
}

bool LoadBlock::realContains(uint64_t raddr) {
    if (mOverlay) {
        return (raddr >= mOverlay->data() && raddr < (mOverlay->data() + size()));
    } else if (mMmap) {
        return (raddr >= mMmap->data() && raddr < (mMmap->data() + size()));
    } else if (oraddr()) {
        return (raddr >= oraddr() && raddr < (oraddr() + size()));
    }
    return false;
}

void LoadBlock::setMmapFile(const char* file, uint64_t offset) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(file, size(), offset));
    if (map) {
        mMmap = std::move(map);
    }
}

void LoadBlock::setOverlay(uint64_t addr, uint64_t value) {
    if (!mOverlay) {
        std::unique_ptr<MemoryMap> map;
        if (isValid()) {
            std::unique_ptr<MemoryMap> tmp(MemoryMap::MmapMem(begin(), size()));
            map = std::move(tmp);
        } else {
            std::unique_ptr<MemoryMap> tmp(MemoryMap::MmapZeroMem(size()));
            map = std::move(tmp);
        }
        if (map) {
            mOverlay = std::move(map);
            LOGI("New overlay [%lx, %lx)\n", vaddr(), vaddr() + size());
        }
    }
    LOGI("Overlay(%lx) Old(%016lx) New(%016lx)\n", addr, *reinterpret_cast<uint64_t *>(mOverlay->data() + (addr - vaddr())), value);
    *reinterpret_cast<uint64_t *>(mOverlay->data() + (addr - vaddr())) = value;
}
