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

#include "base/memory_map.h"
#include "base/utils.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>

MemoryMap* MemoryMap::MmapFile(const char* file) {
    return MmapFile(file, 0);
}

MemoryMap* MemoryMap::MmapFile(const char* file, uint64_t off) {
    struct stat sb;
    if (stat(file, &sb) == -1)
        return nullptr;

    if (off >= sb.st_size)
        return nullptr;

    return MmapFile(file, sb.st_size - off, off);
}

MemoryMap* MemoryMap::MmapFile(const char* file, uint64_t size, uint64_t off) {
    int fd = open(file, O_RDONLY);
    if (fd == -1)
        return nullptr;

    MemoryMap *map = MmapFile(fd, size, off);
    close(fd);
    if (map) map->setFile(file, off);
    return map;
}

MemoryMap* MemoryMap::MmapFile(int fd, uint64_t size, uint64_t off) {
    MemoryMap *map = nullptr;
    struct stat sb;
    if (fd > 0) {
        if (fstat(fd, &sb) == -1)
            return nullptr;

        if (off >= sb.st_size)
            return nullptr;

        void* mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, off);
        if (mem != MAP_FAILED) {
            uint64_t real_size = std::min(size, sb.st_size - off);
            map = new MemoryMap(mem, size, off, real_size);
        }
    }
    return map;
}

MemoryMap* MemoryMap::MmapMem(uint64_t addr, uint64_t size) {
    return MmapMem(addr, size, size);
}

MemoryMap* MemoryMap::MmapMem(uint64_t addr, uint64_t size, uint64_t realSize) {
    MemoryMap *map = nullptr;
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
    if (mem != MAP_FAILED) {
        map = new MemoryMap(mem, size, 0, size);
        memcpy(mem, reinterpret_cast<void *>(addr), realSize);
    }
    return map;
}

MemoryMap* MemoryMap::MmapZeroMem(uint64_t size) {
    MemoryMap *map = nullptr;
    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
    if (mem != MAP_FAILED) {
        map = new MemoryMap(mem, size, 0, size);
        memset(mem, 0x0, size);
    }
    return map;
}

void MemoryMap::setFile(const char* file, uint64_t off) {
    if (file)
        mName = file;
    mOffset = off;
}

uint32_t MemoryMap::GetCRC32() {
    if (!mCRC32) {
        mCRC32 = Utils::CRC32(reinterpret_cast<uint8_t *>(mBegin), mSize);
    }
    return mCRC32;
}

MemoryMap::~MemoryMap() {
    if (mBegin != MAP_FAILED) {
        munmap(mBegin, mSize);
    }
}
