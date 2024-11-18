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
#include "common/syment.h"
#include "base/memory_map.h"
#include "base/macros.h"
#include <string>
#include <memory>
#include <unordered_set>

class LinkMap;

class LoadBlock : public Block {
public:
    inline uint64_t begin() { return begin(OPT_READ_ALL); }
    inline uint64_t begin(int opt) {
        if (UNLIKELY(mOverlay && (opt & OPT_READ_OVERLAY)))
            return mOverlay->data();
        if (UNLIKELY(mMmap && (opt & OPT_READ_MMAP)))
            return mMmap->data();
        if (LIKELY(oraddr() && (opt & OPT_READ_OR)))
            return oraddr();
        return 0x0;
    }
    inline bool isValid() {
        if ((isValidBlock() || mMmap || mOverlay))
            return true;
        return false;
    }
    inline bool virtualContains(uint64_t addr) {
        uint64_t clocaddr = addr & mVabitsMask;
        if ((clocaddr < vaddr())
                || (clocaddr >= (vaddr() + size())))
            return false;
        return true;
    }
    inline bool realContains(uint64_t raddr) {
        if (mOverlay) {
            return (raddr >= mOverlay->data() && raddr < (mOverlay->data() + size()));
        } else if (mMmap) {
            return (raddr >= mMmap->data() && raddr < (mMmap->data() + size()));
        } else if (oraddr()) {
            return (raddr >= oraddr() && raddr < (oraddr() + size()));
        }
        return false;
    }

    LoadBlock(uint32_t f, uint64_t off, uint64_t va, uint64_t pa,
            uint64_t filesz, uint64_t memsz, uint64_t align)
            : Block(f, off, va, pa, filesz, memsz, align) {
        mVabitsMask = 0x0;
        mPointMask = 0x0;
        mCRC32 = 0x0;
        mLinkMap = nullptr;
    }

    void setMmapFile(const char* file, uint64_t offset);
    void setOverlay(uint64_t addr, void *buf, uint64_t size);
    bool newOverlay();
    void removeMmap();
    void removeOverlay();
    inline bool isMmapBlock() { return mMmap != nullptr; }
    inline std::string& name() { return mMmap->getName(); }
    inline void setVabitsMask(uint64_t mask) { mVabitsMask = mask; }
    inline void setPointMask(uint64_t mask) { mPointMask = mask; }
    inline uint64_t VabitsMask() { return mVabitsMask; }
    inline uint64_t PointMask() { return mPointMask; }
    inline uint64_t GetMmapOffset() { return mMmap->offset(); }
    inline void setMmapMemoryMap(std::unique_ptr<MemoryMap>& map) { mMmap = std::move(map); }
    inline std::unordered_set<SymbolEntry, SymbolEntry::Hash>& GetSymbols() { return mSymbols; }
    bool CheckCanMmap(uint64_t header);
    uint32_t GetCRC32(int opt);
    void bind(LinkMap* map) { mLinkMap = map; }
    LinkMap* handle() { return mLinkMap; }

    ~LoadBlock() {
        mSymbols.clear();
        mMmap.reset();
    }
private:
    uint64_t mVabitsMask;
    uint64_t mPointMask;
    uint32_t mCRC32;
    LinkMap* mLinkMap;
    std::unique_ptr<MemoryMap> mMmap;
    std::unordered_set<SymbolEntry, SymbolEntry::Hash> mSymbols;
};

#endif  // CORE_COMMON_LOAD_BLOCK_H_
