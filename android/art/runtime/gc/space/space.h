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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_SPACE_H_

#include "logger/log.h"
#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include <functional>

struct Space_OffsetTable {
    uint32_t vtbl;
    uint32_t vtbl_GetType;
    uint32_t vtbl_IsDlMallocSpace;
    uint32_t vtbl_IsRosAllocSpace;
    uint32_t name_;
};

extern struct Space_OffsetTable __Space_offset__;

struct ContinuousSpace_OffsetTable {
    uint32_t begin_;
    uint32_t end_;
    uint32_t limit_;
};

extern struct ContinuousSpace_OffsetTable __ContinuousSpace_offset__;

struct ContinuousMemMapAllocSpace_OffsetTable {
    uint32_t live_bitmap_;
    uint32_t mark_bitmap_;
    uint32_t temp_bitmap_;
};

extern struct ContinuousMemMapAllocSpace_OffsetTable __ContinuousMemMapAllocSpace_offset__;

namespace art {
namespace gc {
namespace space {

enum SpaceType {
    kSpaceTypeFakeSpace = -2,
    kSpaceTypeInvalidSpace = -1,
    kSpaceTypeImageSpace,
    kSpaceTypeMallocSpace,
    kSpaceTypeZygoteSpace,
    kSpaceTypeBumpPointerSpace,
    kSpaceTypeLargeObjectSpace,
    kSpaceTypeRegionSpace,
};

class Space : public api::MemoryRef {
public:
    inline static const char* FAKE_SPACE = "Fake space";
    inline static const char* REGION_SPACE = "main space (region space)";
    inline static const char* ZYGOTE_SPACE = "Zygote space";
    inline static const char* NON_MOVING_SPACE = "non moving space";
    inline static const char* ROSALLOC_SPACE = "main rosalloc space";
    inline static const char* FREELIST_SPACE = "free list large object space";
    inline static const char* MEMMAP_SPACE = "mem map large object space";
    inline static const char* BUMP_POINTER_SPACE = "Bump pointer space";

    Space(uint64_t v) : api::MemoryRef(v) {}
    Space(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    Space(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Space(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Space(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(Space& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Space& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t vtbl() { return VALUEOF(Space, vtbl); }
    inline uint64_t name() { return Ptr() + OFFSET(Space, name_); }

    const char* GetName();
    inline bool IsFakeSpace() { return GetType() == kSpaceTypeFakeSpace; }
    inline bool IsImageSpace() { return GetType() == kSpaceTypeImageSpace; }
    inline bool IsMallocSpace() { return GetType() == kSpaceTypeMallocSpace; }
    inline bool IsZygoteSpace() { return GetType() == kSpaceTypeZygoteSpace; }
    inline bool IsBumpPointerSpace() { return GetType() == kSpaceTypeBumpPointerSpace; }
    inline bool IsLargeObjectSpace() { return GetType() == kSpaceTypeLargeObjectSpace; }
    inline bool IsRegionSpace() { return GetType() == kSpaceTypeRegionSpace; }

    virtual SpaceType GetType();
    virtual bool IsRosAllocSpace();
    virtual bool IsDlMallocSpace();
    bool GetXMallocSpaceFlag(uint32_t off);
    virtual void Walk(std::function<bool (mirror::Object& object)> fn, bool check) {}
    virtual bool IsVaildSpace() { return false; }
private:
    SpaceType type_cache = kSpaceTypeInvalidSpace;
    // quick memoryref cache
    api::MemoryRef vtbl_cache = 0x0;
};

class ContinuousSpace : public Space {
public:
    ContinuousSpace(uint64_t v) : Space(v) {}
    ContinuousSpace(uint64_t v, LoadBlock* b) : Space(v, b) {}
    ContinuousSpace(const Space& ref) : Space(ref) {}
    ContinuousSpace(uint64_t v, Space& ref) : Space(v, ref) {}
    ContinuousSpace(uint64_t v, Space* ref) : Space(v, ref) {}

    static void Init();
    inline uint64_t begin() { return VALUEOF(ContinuousSpace, begin_); }
    inline uint64_t end() { return VALUEOF(ContinuousSpace, end_); }
    inline uint64_t limit() { return VALUEOF(ContinuousSpace, limit_); }

    inline uint64_t Begin() { return begin(); }
    inline uint64_t End() { return end(); }
    inline uint64_t Limit() { return limit(); }
    uint64_t GetNextObject(mirror::Object& object);
    bool IsVaildSpace();
};

class DiscontinuousSpace : public Space {
public:
    DiscontinuousSpace(uint64_t v) : Space(v) {}
    DiscontinuousSpace(uint64_t v, LoadBlock* b) : Space(v, b) {}
    DiscontinuousSpace(const Space& ref) : Space(ref) {}
    DiscontinuousSpace(uint64_t v, Space& ref) : Space(v, ref) {}
    DiscontinuousSpace(uint64_t v, Space* ref) : Space(v, ref) {}
};

class MemMapSpace : public ContinuousSpace {
public:
    MemMapSpace(uint64_t v) : ContinuousSpace(v) {}
    MemMapSpace(uint64_t v, LoadBlock* b) : ContinuousSpace(v, b) {}
    MemMapSpace(const ContinuousSpace& ref) : ContinuousSpace(ref) {}
    MemMapSpace(uint64_t v, ContinuousSpace& ref) : ContinuousSpace(v, ref) {}
    MemMapSpace(uint64_t v, ContinuousSpace* ref) : ContinuousSpace(v, ref) {}
};

class AllocSpace {};

class ContinuousMemMapAllocSpace : public MemMapSpace, AllocSpace {
public:
    ContinuousMemMapAllocSpace(uint64_t v) : MemMapSpace(v) {}
    ContinuousMemMapAllocSpace(uint64_t v, LoadBlock* b) : MemMapSpace(v, b) {}
    ContinuousMemMapAllocSpace(const MemMapSpace& ref) : MemMapSpace(ref) {}
    ContinuousMemMapAllocSpace(uint64_t v, MemMapSpace& ref) : MemMapSpace(v, ref) {}
    ContinuousMemMapAllocSpace(uint64_t v, MemMapSpace* ref) : MemMapSpace(v, ref) {}

    static void Init();
    static void Init36();
    inline uint64_t live_bitmap() { return Ptr() + OFFSET(ContinuousMemMapAllocSpace, live_bitmap_); }
    inline uint64_t mark_bitmap() { return Ptr() + OFFSET(ContinuousMemMapAllocSpace, mark_bitmap_); }
    inline uint64_t temp_bitmap() { return Ptr() + OFFSET(ContinuousMemMapAllocSpace, temp_bitmap_); }
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_SPACE_H_
