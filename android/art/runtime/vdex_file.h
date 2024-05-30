/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_ART_RUNTIME_VDEX_FILE_H_
#define ANDROID_ART_RUNTIME_VDEX_FILE_H_

#include "api/memory_ref.h"
#include "base/mem_map.h"

struct VdexFile_OffsetTable {
    uint32_t mmap_;
};
extern struct VdexFile_OffsetTable __VdexFile_offset__;

namespace art {

class VdexFile : public api::MemoryRef {
public:
    VdexFile(uint64_t v) : api::MemoryRef(v) {}
    VdexFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    VdexFile(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    VdexFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> VdexFile(U *v) : api::MemoryRef(v) {}
    template<typename U> VdexFile(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t mmap() { return Ptr() + OFFSET(VdexFile, mmap_); }
    inline uint64_t mmap_lv29() { return VALUEOF(VdexFile, mmap_); }

    MemMap& GetMap();
    const char* GetName();
    uint64_t Begin();
private:
    // quick memoryref cache
    MemMap mmap_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_VDEX_FILE_H_
