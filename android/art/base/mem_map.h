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

#ifndef ANDROID_ART_BASE_MEM_MAP_H_
#define ANDROID_ART_BASE_MEM_MAP_H_

#include "api/memory_ref.h"
#include "cxx/string.h"

struct MemMap_OffsetTable {
    uint32_t name_;
    uint32_t begin_;
    uint32_t size_;
};

struct MemMap_SizeTable {
    uint32_t THIS;
};

extern struct MemMap_OffsetTable __MemMap_offset__;
extern struct MemMap_SizeTable __MemMap_size__;

namespace art {

class MemMap : public api::MemoryRef {
public:
    MemMap(uint64_t v) : api::MemoryRef(v) {}
    MemMap(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    MemMap(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t name() { return Ptr() + OFFSET(MemMap, name_); }
    inline uint64_t begin() { return VALUEOF(MemMap, begin_); }
    inline uint64_t size() { return VALUEOF(MemMap, size_); }

    const char* GetName();
    bool HasAddress(uint64_t addr) {
        return begin() <= addr && addr < begin() + size();
    }
private:
    // quick memoryref cache
    cxx::string name_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_BASE_MEM_MAP_H_
