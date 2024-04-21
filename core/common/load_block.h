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

#ifndef CORE_COMMON_LOAD_BLOCK_H_
#define CORE_COMMON_LOAD_BLOCK_H_

#include "common/block.h"
#include "base/memory_map.h"
#include <string>
#include <memory>

class LoadBlock : public Block {
public:
    static constexpr int OPT_READ_OR = (1 << 0);
    static constexpr int OPT_READ_MMAP = (1 << 1);
    static constexpr int OPT_READ_OVERLAY = (1 << 2);
    static constexpr int OPT_READ_ALL = OPT_READ_OR | OPT_READ_MMAP | OPT_READ_OVERLAY;

    uint64_t begin() { return begin(OPT_READ_ALL); }
    uint64_t begin(int opt);
    bool isValid();
    bool virtualContains(uint64_t vaddr);
    bool realContains(uint64_t raddr);

    LoadBlock(uint32_t f, uint64_t off, uint64_t va, uint64_t pa,
            uint64_t filesz, uint64_t memsz, uint64_t align)
            : Block(f, off, va, pa, filesz, memsz, align) {}

    void setMmapFile(const char* file, uint64_t offset);
    void setOverlay(uint64_t addr, uint64_t value);
    void removeMmap();
    void removeOverlay();
    bool isMmapBlock() { return mMmap != nullptr; }
    bool isOverlayBlock() { return mOverlay != nullptr; }
    inline std::string& name() { return mMmap->getName(); }
    void setVabitsMask(uint64_t mask) { mVabitsMask = mask; }
    void setPointMask(uint64_t mask) { mPointMask = mask; }
    inline uint64_t VabitsMask() { return mVabitsMask; }
    inline uint64_t PointMask() { return mPointMask; }
    inline uint64_t GetMmapOffset() { return mMmap->offset(); }

    ~LoadBlock() {
        mOverlay.reset();
        mMmap.reset();
    }
private:
    uint64_t mVabitsMask;
    uint64_t mPointMask;
    std::unique_ptr<MemoryMap> mMmap;
    std::unique_ptr<MemoryMap> mOverlay;
};

#endif  // CORE_COMMON_LOAD_BLOCK_H_
