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

#ifndef UTILS_BASE_MEM_MAP_H_
#define UTILS_BASE_MEM_MAP_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>

class MemMap {
public:
    static MemMap* MmapFile(const char* file);
    void* data() { return mem; }
    std::string getFile() { return file_path; }
    ~MemMap();
private:
    static MemMap* MmapFile(int fd, uint64_t size, uint64_t off);
    MemMap(void *m, uint64_t s, uint64_t off)
        : mem(m), size(s) {}

    void* mem;
    uint64_t size;
    uint64_t off;
    std::string file_path;
};

#endif  // UTILS_BASE_MEM_MAP_H_
