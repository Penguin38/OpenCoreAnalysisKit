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

#ifndef UTILS_BASE_MEMORY_MAP_H_
#define UTILS_BASE_MEMORY_MAP_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>

class MemoryMap {
public:
    static MemoryMap* MmapFile(const char* file);
    static MemoryMap* MmapFile(const char* file, uint64_t off);
    static MemoryMap* MmapFile(const char* file, uint64_t size, uint64_t off);
    static MemoryMap* MmapMem(uint64_t addr, uint64_t size);
    static MemoryMap* MmapZeroMem(uint64_t size);
    inline uint64_t data() { return reinterpret_cast<uint64_t>(mBegin); }
    inline uint64_t size() { return mSize; }
    inline uint64_t offset() { return mOffset; }
    inline std::string& getName() { return mName; }
    uint32_t GetCRC32();
    ~MemoryMap();
private:
    static MemoryMap* MmapFile(int fd, uint64_t size, uint64_t off);
    MemoryMap(void *m, uint64_t s, uint64_t off)
        : mBegin(m), mSize(s), mOffset(off), mCRC32(0x0) {}

    std::string mName;
    void* mBegin;
    uint64_t mSize;
    uint64_t mOffset;
    uint32_t mCRC32;
};

#endif  // UTILS_BASE_MEMORY_MAP_H_
