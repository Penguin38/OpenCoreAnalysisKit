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

void LoadBlock::setMmapFile(const char* file, uint64_t offset) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(file, size(), offset));
    if (map) {
        mMmap = std::move(map);
        LOGI("Mmap segment [%lx, %lx) %s [%lx]\n", vaddr(), vaddr() + size(), file, offset);
    }
}

bool LoadBlock::newOverlay() {
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
    return mOverlay != 0x0;
}

void LoadBlock::setOverlay(uint64_t addr, void *buf, uint64_t size) {
    if (newOverlay()) {
        uint64_t clocaddr = addr & mVabitsMask;
        memcpy(reinterpret_cast<uint64_t *>(mOverlay->data() + (clocaddr - vaddr())), buf, size);
    }
}

void LoadBlock::removeMmap() {
    if (mMmap) {
        LOGI("Remove mmap [%lx, %lx) %s\n", vaddr(), vaddr() + size(), name().c_str());
        mSymbols.clear();
        mMmap.reset();
    }
}

void LoadBlock::removeOverlay() {
    if (mOverlay) {
        if (!isFake()) {
            LOGI("Remove overlay [%lx, %lx)\n", vaddr(), vaddr() + size());
            mOverlay.reset();
        } else {
            LOGE("Can't remove fake load\n");
        }
    }
}
