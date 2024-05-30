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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_LARGE_OBJECT_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_LARGE_OBJECT_SPACE_H_

#include "runtime/gc/space/space.h"
#include "cxx/map.h"

struct LargeObjectSpace_OffsetTable {
    uint32_t lock_;
    uint32_t num_bytes_allocated_;
    uint32_t num_objects_allocated_;
    uint32_t total_bytes_allocated_;
    uint32_t total_objects_allocated_;
    uint32_t begin_;
    uint32_t end_;
};

struct LargeObjectSpace_SizeTable {
    uint32_t THIS;
};

extern struct LargeObjectSpace_OffsetTable __LargeObjectSpace_offset__;
extern struct LargeObjectSpace_SizeTable __LargeObjectSpace_size__;

struct LargeObjectMapSpace_OffsetTable {
    uint32_t large_objects_;
};

struct LargeObjectMapSpace_SizeTable {
    uint32_t THIS;
};

extern struct LargeObjectMapSpace_OffsetTable __LargeObjectMapSpace_offset__;
extern struct LargeObjectMapSpace_SizeTable __LargeObjectMapSpace_size__;

struct LargeObject_OffsetTable {
    uint32_t mem_map;
    uint32_t is_zygote;
};

struct LargeObject_SizeTable {
    uint32_t THIS;
};

extern struct LargeObject_OffsetTable __LargeObject_offset__;
extern struct LargeObject_SizeTable __LargeObject_size__;

struct LargeObjectsPair_OffsetTable {
    uint32_t first;
    uint32_t second;
};

struct LargeObjectsPair_SizeTable {
    uint32_t THIS;
};

extern struct LargeObjectsPair_OffsetTable __LargeObjectsPair_offset__;
extern struct LargeObjectsPair_SizeTable __LargeObjectsPair_size__;

struct AllocationInfo_OffsetTable {
    uint32_t prev_free_;
    uint32_t alloc_size_;
};

struct AllocationInfo_SizeTable {
    uint32_t THIS;
};

extern struct AllocationInfo_OffsetTable __AllocationInfo_offset__;
extern struct AllocationInfo_SizeTable __AllocationInfo_size__;

struct FreeListSpace_OffsetTable {
    uint32_t mem_map_;
    uint32_t allocation_info_map_;
    uint32_t allocation_info_;
    uint32_t free_end_;
    uint32_t free_blocks_;
};

struct FreeListSpace_SizeTable {
    uint32_t THIS;
};

extern struct FreeListSpace_OffsetTable __FreeListSpace_offset__;
extern struct FreeListSpace_SizeTable __FreeListSpace_size__;

namespace art {
namespace gc {
namespace space {

class LargeObjectSpace : public DiscontinuousSpace, AllocSpace {
public:
    LargeObjectSpace() : DiscontinuousSpace() {}
    LargeObjectSpace(uint64_t v) : DiscontinuousSpace(v) {}
    LargeObjectSpace(uint64_t v, LoadBlock* b) : DiscontinuousSpace(v, b) {}
    LargeObjectSpace(const DiscontinuousSpace& ref) : DiscontinuousSpace(ref) {}
    LargeObjectSpace(uint64_t v, DiscontinuousSpace& ref) : DiscontinuousSpace(v, ref) {}
    LargeObjectSpace(uint64_t v, DiscontinuousSpace* ref) : DiscontinuousSpace(v, ref) {}
    template<typename U> LargeObjectSpace(U *v) : DiscontinuousSpace(v) {}
    template<typename U> LargeObjectSpace(U *v, DiscontinuousSpace* ref) : DiscontinuousSpace(v, ref) {}

    static void Init26();
    static void Init29();
    static void Init30();
    inline uint64_t begin() { return VALUEOF(LargeObjectSpace, begin_); }
    inline uint64_t end() { return VALUEOF(LargeObjectSpace, end_); }

    inline uint64_t Begin() { return begin(); }
    inline uint64_t End() { return end(); }
    inline bool Contains(mirror::Object& object) { return begin() <= object.Ptr() && object.Ptr() < end(); }
    bool IsFreeListSpace();
    bool IsMemMapSpace();
    SpaceType GetType() { return kSpaceTypeLargeObjectSpace; }
};

class LargeObjectMapSpace : public LargeObjectSpace {
public:
    LargeObjectMapSpace() : LargeObjectSpace() {}
    LargeObjectMapSpace(uint64_t v) : LargeObjectSpace(v) {}
    LargeObjectMapSpace(uint64_t v, LoadBlock* b) : LargeObjectSpace(v, b) {}
    LargeObjectMapSpace(const LargeObjectSpace& ref) : LargeObjectSpace(ref) {}
    LargeObjectMapSpace(uint64_t v, LargeObjectSpace& ref) : LargeObjectSpace(v, ref) {}
    LargeObjectMapSpace(uint64_t v, LargeObjectSpace* ref) : LargeObjectSpace(v, ref) {}
    template<typename U> LargeObjectMapSpace(U *v) : LargeObjectSpace(v) {}
    template<typename U> LargeObjectMapSpace(U *v, LargeObjectSpace* ref) : LargeObjectSpace(v, ref) {}

    static void Init26();
    static void Init28();
    static void Init29();
    static void Init30();
    inline uint64_t large_objects() { return Ptr() + OFFSET(LargeObjectMapSpace, large_objects_); }

    cxx::map& GetLargeObjectsCache();
    void Walk(std::function<bool (mirror::Object& object)> fn);
    bool IsVaildSpace();

    class LargeObject : public api::MemoryRef {
    public:
        LargeObject(uint64_t v) : api::MemoryRef(v) {}
        LargeObject(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        LargeObject(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        LargeObject(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        template<typename U> LargeObject(U *v) : api::MemoryRef(v) {}
        template<typename U> LargeObject(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init();
    };

    class LargeObjectsPair : public api::MemoryRef {
    public:
        LargeObjectsPair(uint64_t v) : api::MemoryRef(v) {}
        LargeObjectsPair(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        LargeObjectsPair(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        LargeObjectsPair(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        template<typename U> LargeObjectsPair(U *v) : api::MemoryRef(v) {}
        template<typename U> LargeObjectsPair(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init();
        inline uint64_t first() { return VALUEOF(LargeObjectsPair, first); }
        inline uint64_t second() { return Ptr() + OFFSET(LargeObjectsPair, second); }
    };

private:
    // quick memoryref cache
    cxx::map large_objects_cache;
};

class AllocationInfo : public api::MemoryRef {
public:
    AllocationInfo(uint64_t v) : api::MemoryRef(v) {}
    AllocationInfo(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    AllocationInfo(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    AllocationInfo(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    template<typename U> AllocationInfo(U *v) : api::MemoryRef(v) {}
    template<typename U> AllocationInfo(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint32_t prev_free() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(AllocationInfo, prev_free_)); }
    inline uint32_t alloc_size() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(AllocationInfo, alloc_size_)); }

    inline bool IsFree() { return (alloc_size() & kFlagFree) != 0; }
    inline uint32_t AlignSize() { return alloc_size() & kFlagsMask; }
    inline void MoveNexInfo() { MovePtr(AlignSize() * SIZEOF(AllocationInfo)); }
private:
    static constexpr uint32_t kFlagFree = 0x80000000;
    static constexpr uint32_t kFlagZygote = 0x40000000;
    static constexpr uint32_t kFlagsMask = ~(kFlagFree | kFlagZygote);
};

class FreeListSpace : public LargeObjectSpace {
public:
    FreeListSpace() : LargeObjectSpace() {}
    FreeListSpace(uint64_t v) : LargeObjectSpace(v) {}
    FreeListSpace(uint64_t v, LoadBlock* b) : LargeObjectSpace(v, b) {}
    FreeListSpace(const LargeObjectSpace& ref) : LargeObjectSpace(ref) {}
    FreeListSpace(uint64_t v, LargeObjectSpace& ref) : LargeObjectSpace(v, ref) {}
    FreeListSpace(uint64_t v, LargeObjectSpace* ref) : LargeObjectSpace(v, ref) {}
    template<typename U> FreeListSpace(U *v) : LargeObjectSpace(v) {}
    template<typename U> FreeListSpace(U *v, LargeObjectSpace* ref) : LargeObjectSpace(v, ref) {}

    static void Init();
    inline uint64_t free_end() { return VALUEOF(FreeListSpace, free_end_); }
    inline uint64_t allocation_info() { return VALUEOF(FreeListSpace, allocation_info_); }

    api::MemoryRef& GetAlloctionInfoCache();
    void Walk(std::function<bool (mirror::Object& object)> fn);
    uint64_t GetAllocationInfoForAddress(uint64_t address);
    uint64_t GetSlotIndexForAddress(uint64_t address);
    uint64_t GetAddressForAllocationInfo(AllocationInfo& info);
    uint64_t GetAllocationAddressForSlot(uint64_t slot);
    bool IsVaildSpace();
private:
    // quick memoryref cache
    api::MemoryRef allocation_info_cache;
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_LARGE_OBJECT_SPACE_H_
