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
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

MemoryMap* MemoryMap::MmapFile(const char* file) {
    int fd;
    struct stat sb;

    fd = open(file, O_RDONLY);
    if (fd == -1)
        return nullptr;

    if (fstat(fd, &sb) == -1)
        return nullptr;

    MemoryMap *map = MmapFile(fd, sb.st_size, 0);
    close(fd);
    if (map) {
        map->mName = file;
    }
    return map;
}

MemoryMap* MemoryMap::MmapFile(int fd, uint64_t size, uint64_t off) {
    MemoryMap *map = nullptr;
    if (fd > 0) {
        void* mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, off);
        if (mem != MAP_FAILED) {
            map = new MemoryMap(mem, size, off);
        }
    }
    return map;
}

MemoryMap::~MemoryMap() {
    if (mBegin != MAP_FAILED) {
        munmap(mBegin, mSize);
    }
    std::cout << __func__ << " " << this << std::endl;
}
