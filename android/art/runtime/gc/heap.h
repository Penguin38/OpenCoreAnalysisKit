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

#ifndef ANDROID_ART_RUNTIME_GC_HEAP_H_
#define ANDROID_ART_RUNTIME_GC_HEAP_H_

#include "logger/log.h"
#include "api/memory_ref.h"
#include "cxx/vector.h"
#include "runtime/gc/space/space.h"
#include <vector>
#include <memory>

struct Heap_OffsetTable {
    uint32_t continuous_spaces_;
    uint32_t discontinuous_spaces_;
};

struct Heap_SizeTable {
    uint32_t THIS;
};

extern struct Heap_OffsetTable __Heap_offset__;
extern struct Heap_SizeTable __Heap_size__;

namespace art {
namespace gc {

class Heap : public api::MemoryRef {
public:
    Heap(uint64_t v) : api::MemoryRef(v) {}
    Heap(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    Heap(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Heap(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Heap(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(Heap& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Heap& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    static void Init23();
    static void Init35();
    inline uint64_t continuous_spaces() { return Ptr() + OFFSET(Heap, continuous_spaces_); }
    inline uint64_t discontinuous_spaces() { return Ptr() + OFFSET(Heap, discontinuous_spaces_); }

    cxx::vector& GetContinuousSpacesCache();
    cxx::vector& GetDiscontinuousSpacesCache();
    std::vector<std::unique_ptr<space::ContinuousSpace>>& GetContinuousSpaces();
    std::vector<std::unique_ptr<space::DiscontinuousSpace>>& GetDiscontinuousSpaces();
    void CleanCache() {
        continuous_spaces_second_cache.clear();
        discontinuous_spaces_second_cache.clear();
    }

    space::ContinuousSpace* FindContinuousSpaceFromObject(mirror::Object& object);
private:
    // quick memoryref cache
    cxx::vector continuous_spaces_cache = 0x0;
    cxx::vector discontinuous_spaces_cache = 0x0;

    // second cache
    std::vector<std::unique_ptr<space::ContinuousSpace>> continuous_spaces_second_cache;
    std::vector<std::unique_ptr<space::DiscontinuousSpace>> discontinuous_spaces_second_cache;
};

} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_HEAP_H_
