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

#ifndef ANDROID_ART_RUNTIME_JIT_JIT_MEMORY_REGION_H_
#define ANDROID_ART_RUNTIME_JIT_JIT_MEMORY_REGION_H_

#include "api/memory_ref.h"
#include "base/mem_map.h"

struct JitMemoryRegion_OffsetTable {
    uint32_t exec_pages_;
};

extern struct JitMemoryRegion_OffsetTable __JitMemoryRegion_offset__;

namespace art {
namespace jit {

class JitMemoryRegion : public api::MemoryRef {
public:
    JitMemoryRegion(uint64_t v) : api::MemoryRef(v) {}
    JitMemoryRegion(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    JitMemoryRegion(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    JitMemoryRegion(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init30();
    inline uint64_t exec_pages() { return Ptr() + OFFSET(JitMemoryRegion, exec_pages_); }

    MemMap& GetExecPages();
    bool IsInExecSpace(uint64_t ptr) {
        return GetExecPages().HasAddress(ptr);
    }
private:
    // quick memoryref cache
    MemMap exec_pages_cache = 0x0;
};

} // namespace jit
} // namespace art

#endif // ANDROID_ART_RUNTIME_JIT_JIT_MEMORY_REGION_H_
