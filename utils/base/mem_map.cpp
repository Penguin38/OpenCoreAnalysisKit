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

#include "base/mem_map.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

MemMap* MemMap::MmapFile(const char* file) {
    int fd;
    struct stat sb;

    fd = open(file, O_RDONLY);
    if (fd == -1)
        return nullptr;

    if (fstat(fd, &sb) == -1)
        return nullptr;

    MemMap *map = MmapFile(fd, sb.st_size, 0);
    close(fd);
    if (map) {
        map->file_path = file;
    }
    return map;
}

MemMap* MemMap::MmapFile(int fd, uint64_t size, uint64_t off) {
    MemMap *map = nullptr;
    if (fd > 0) {
        void* mem = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, off);
        if (mem != MAP_FAILED) {
            map = new MemMap(mem, size, off);
        }
    }
    return map;
}

MemMap::~MemMap() {
    if (mem != MAP_FAILED) {
        munmap(mem, size);
    }
}
