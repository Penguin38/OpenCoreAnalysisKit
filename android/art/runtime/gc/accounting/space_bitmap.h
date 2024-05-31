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

#ifndef ANDROID_ART_RUNTIME_GC_ACCOUNTING_SPACE_BITMAP_H_
#define ANDROID_ART_RUNTIME_GC_ACCOUNTING_SPACE_BITMAP_H_

#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include <functional>

struct ContinuousSpaceBitmap_OffsetTable {
    uint32_t mem_map_;
    uint32_t bitmap_begin_;
    uint32_t bitmap_size_;
    uint32_t heap_begin_;
};

extern struct ContinuousSpaceBitmap_OffsetTable __ContinuousSpaceBitmap_offset__;

namespace art {
namespace gc {
namespace accounting {

class ContinuousSpaceBitmap : public api::MemoryRef {
public:
    ContinuousSpaceBitmap() : api::MemoryRef() {}
    ContinuousSpaceBitmap(uint64_t v) : api::MemoryRef(v) {}
    ContinuousSpaceBitmap(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    ContinuousSpaceBitmap(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ContinuousSpaceBitmap(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ContinuousSpaceBitmap(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> ContinuousSpaceBitmap(U *v) : api::MemoryRef(v) {}
    template<typename U> ContinuousSpaceBitmap(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init26();
    static void Init29();
    inline uint64_t bitmap_begin() { return VALUEOF(ContinuousSpaceBitmap, bitmap_begin_); }
    inline uint64_t bitmap_size() { return VALUEOF(ContinuousSpaceBitmap, bitmap_size_); }
    inline uint64_t heap_begin() { return VALUEOF(ContinuousSpaceBitmap, heap_begin_); }

    void VisitMarkedRange(uint64_t visit_begin, uint64_t visit_end, std::function<bool (mirror::Object& object)> fn);
    uint64_t OffsetToIndex(uint64_t offset, int point_bit);
    uint64_t IndexToOffset(uint64_t index, int point_bit);
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_ACCOUNTING_SPACE_BITMAP_H_
