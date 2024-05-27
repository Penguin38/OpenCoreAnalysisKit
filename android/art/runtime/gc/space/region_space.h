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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_REGION_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_REGION_SPACE_H_

#include "runtime/gc/space/space.h"
#include "runtime/mirror/object.h"
#include "runtime/gc/accounting/space_bitmap.h"
#include <functional>

struct RegionSpace_OffsetTable {
    uint32_t num_regions_;
    uint32_t regions_;
    uint32_t mark_bitmap_;
};

struct RegionSpace_SizeTable {
    uint32_t THIS;
};

extern struct RegionSpace_OffsetTable __RegionSpace_offset__;
extern struct RegionSpace_SizeTable __RegionSpace_size__;

struct Region_OffsetTable {
    uint32_t idx_;
    uint32_t live_bytes_;
    uint32_t begin_;
    uint32_t thread_;
    uint32_t top_;
    uint32_t end_;
    uint32_t objects_allocated_;
    uint32_t alloc_time_;
    uint32_t is_newly_allocated_;
    uint32_t is_a_tlab_;
    uint32_t state_;
    uint32_t type_;
};

struct Region_SizeTable {
    uint32_t THIS;
};

extern struct Region_OffsetTable __Region_offset__;
extern struct Region_SizeTable __Region_size__;

namespace art {
namespace gc {
namespace space {

class RegionSpace : public ContinuousMemMapAllocSpace {
public:
    RegionSpace() : ContinuousMemMapAllocSpace() {}
    RegionSpace(uint64_t v) : ContinuousMemMapAllocSpace(v) {}
    RegionSpace(uint64_t v, LoadBlock* b) : ContinuousMemMapAllocSpace(v, b) {}
    RegionSpace(const ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(ref) {}
    RegionSpace(uint64_t v, ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(v, ref) {}
    RegionSpace(uint64_t v, ContinuousMemMapAllocSpace* ref) : ContinuousMemMapAllocSpace(v, ref) {}
    template<typename U> RegionSpace(U *v) : ContinuousMemMapAllocSpace(v) {}
    template<typename U> RegionSpace(U *v, ContinuousMemMapAllocSpace* ref) : ContinuousMemMapAllocSpace(v, ref) {}

    static void Init29();
    static void Init();
    static void Init31();
    inline uint64_t num_regions() { return VALUEOF(RegionSpace, num_regions_); }
    inline uint64_t regions() { return VALUEOF(RegionSpace, regions_); }
    inline uint64_t mark_bitmap() { return Ptr() + OFFSET(RegionSpace, mark_bitmap_); }

    SpaceType GetType() { return kSpaceTypeRegionSpace; }
    void Walk(std::function<bool (mirror::Object& object)> fn);
    void WalkInternal(std::function<bool (mirror::Object& object)> fn, bool only);

    enum class RegionType : uint8_t {
        kRegionTypeAll,              // All types.
        kRegionTypeFromSpace,        // From-space. To be evacuated.
        kRegionTypeUnevacFromSpace,  // Unevacuated from-space. Not to be evacuated.
        kRegionTypeToSpace,          // To-space.
        kRegionTypeNone,             // None.
    };

    enum class RegionState : uint8_t {
        kRegionStateFree,            // Free region.
        kRegionStateAllocated,       // Allocated region.
        kRegionStateLarge,           // Large allocated (allocation larger than the region size).
        kRegionStateLargeTail,       // Large tail (non-first regions of a large allocation).
    };

    class Region : public api::MemoryRef {
    public:
        Region() : api::MemoryRef() {}
        Region(uint64_t v) : api::MemoryRef(v) {}
        Region(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
        Region(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        Region(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        Region(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        template<typename U> Region(U *v) : api::MemoryRef(v) {}
        template<typename U> Region(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init();
        inline uint64_t live_bytes() { return VALUEOF(Region, live_bytes_); }
        inline uint64_t begin() { return VALUEOF(Region, begin_); }
        inline uint64_t top() { return VALUEOF(Region, top_); }
        inline uint64_t objects_allocated() { return VALUEOF(Region, objects_allocated_); }
        inline uint8_t state() { return *reinterpret_cast<uint8_t*>(Real() + OFFSET(Region, state_)); }
        inline uint8_t type() { return *reinterpret_cast<uint8_t*>(Real() + OFFSET(Region, type_)); }

        inline bool IsFree() { return state() == static_cast<uint8_t>(RegionState::kRegionStateFree); }
        inline bool IsInToSpace() { return type() == static_cast<uint8_t>(RegionType::kRegionTypeToSpace); }
        inline bool IsLarge() { return state() == static_cast<uint8_t>(RegionState::kRegionStateLarge); }
        inline bool IsLargeTail() { return state() == static_cast<uint8_t>(RegionState::kRegionStateLargeTail); }
        inline uint64_t Begin() { return begin(); }
        inline uint64_t Top() { return top(); }
        inline uint64_t LiveBytes() { return live_bytes(); }
        inline uint64_t ObjectsAllocated() { return objects_allocated(); }
    };

    void WalkNonLargeRegion(std::function<bool (mirror::Object& object)> fn, RegionSpace::Region& region);
    accounting::ContinuousSpaceBitmap& GetLiveBitmap();

private:
    // quick memoryref cache
    accounting::ContinuousSpaceBitmap mark_bitmap_cache;
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_REGION_SPACE_H_
