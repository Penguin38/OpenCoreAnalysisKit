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

#ifndef CORE_COMMON_BLOCK_H_
#define CORE_COMMON_BLOCK_H_

#include "base/memory_map.h"
#include <stdint.h>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <memory>

class Block {
public:
    static constexpr int FLAG_X = (1 << 0);    /* Segment is executable */
    static constexpr int FLAG_W = (1 << 1);    /* Segment is writable */
    static constexpr int FLAG_R = (1 << 2);    /* Segment is readable */

    static constexpr int OPT_READ_OR = (1 << 0);
    static constexpr int OPT_READ_MMAP = (1 << 1);
    static constexpr int OPT_READ_OVERLAY = (1 << 2);
    static constexpr int OPT_READ_ALL = OPT_READ_OR | OPT_READ_MMAP | OPT_READ_OVERLAY;

    inline uint64_t vaddr() { return mVaddr; }
    inline uint64_t paddr() { return mPaddr; }
    inline uint32_t flags() { return mFlags; }
    inline uint64_t offset() { return mOffset; }
    inline uint64_t size() { return mMemSize; }
    inline uint64_t realSize() { return mFileSize; }
    inline uint64_t align() { return mAlign; }
    inline bool isValidBlock() { return !mTruncated && (mFileSize > 0); }
    inline uint64_t oraddr() { return mOriAddr; }
    inline bool isFake() { return mFake; }

    Block(uint32_t f, uint64_t off, uint64_t va, uint64_t pa,
            uint64_t filesz, uint64_t memsz, uint64_t align)
            : mFlags(f), mOffset(off), mVaddr(va), mPaddr(pa),
              mFileSize(filesz), mMemSize(memsz), mAlign(align),
              mOriAddr(0x0), mTruncated(false), mFake(false) {}

    ~Block() {
        mOverlay.reset();
    }
    void setOriAddr(uint64_t addr) { if (isValidBlock()) mOriAddr = addr; }
    void setTruncated(bool truncated) { mTruncated = truncated; }
    void setFake(bool fake) { mFake = fake; }

    inline std::string convertFlags() {
        std::string sb;
        if (flags() & FLAG_R) {
            sb.append("r");
        } else {
            sb.append("-");
        }

        if (flags() & FLAG_W) {
            sb.append("w");
        } else {
            sb.append("-");
        }

        if (flags() & FLAG_X) {
            sb.append("x");
        } else {
            sb.append("-");
        }
        return sb;
    }

    virtual bool newOverlay() { return false; }
    virtual void setOverlay(uint64_t addr, void *buf, uint64_t size) {}
    virtual void removeOverlay() {}
    inline bool isOverlayBlock() { return mOverlay != nullptr; }
    inline void setOverlayMemoryMap(std::unique_ptr<MemoryMap>& map) { mOverlay = std::move(map); }
protected:
    std::unique_ptr<MemoryMap> mOverlay;

private:
    //  program member
    uint32_t mFlags;
    uint64_t mOffset;
    uint64_t mVaddr;
    uint64_t mPaddr;
    uint64_t mFileSize;
    uint64_t mMemSize;
    uint64_t mAlign;

    // Real memory addr
    uint64_t mOriAddr;
    bool mTruncated;
    bool mFake;
};

#endif  // CORE_COMMON_BLOCK_H_
