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

#include "api/core.h"
#include "android.h"
#include "runtime/gc/space/large_object_space.h"
#include "runtime/runtime_globals.h"
#include <string.h>

struct LargeObjectSpace_OffsetTable __LargeObjectSpace_offset__;
struct LargeObjectSpace_SizeTable __LargeObjectSpace_size__;
struct LargeObjectMapSpace_OffsetTable __LargeObjectMapSpace_offset__;
struct LargeObjectMapSpace_SizeTable __LargeObjectMapSpace_size__;
struct LargeObject_OffsetTable __LargeObject_offset__;
struct LargeObject_SizeTable __LargeObject_size__;
struct LargeObjectsPair_OffsetTable __LargeObjectsPair_offset__;
struct LargeObjectsPair_SizeTable __LargeObjectsPair_size__;
struct AllocationInfo_OffsetTable __AllocationInfo_offset__;
struct AllocationInfo_SizeTable __AllocationInfo_size__;
struct FreeListSpace_OffsetTable __FreeListSpace_offset__;
struct FreeListSpace_SizeTable __FreeListSpace_size__;

namespace art {
namespace gc {
namespace space {

void LargeObjectSpace::Init() {
    art::gc::space::LargeObjectMapSpace::LargeObject::Init();
    art::gc::space::LargeObjectMapSpace::LargeObjectsPair::Init();
    art::gc::space::AllocationInfo::Init();

    Android::RegisterSdkListener(Android::M, art::gc::space::LargeObjectSpace::Init23);
    Android::RegisterSdkListener(Android::Q, art::gc::space::LargeObjectSpace::Init29);
    Android::RegisterSdkListener(Android::R, art::gc::space::LargeObjectSpace::Init30);

    Android::RegisterSdkListener(Android::M, art::gc::space::LargeObjectMapSpace::Init23);
    Android::RegisterSdkListener(Android::N, art::gc::space::LargeObjectMapSpace::Init24);
    Android::RegisterSdkListener(Android::O, art::gc::space::LargeObjectMapSpace::Init26);
    Android::RegisterSdkListener(Android::P, art::gc::space::LargeObjectMapSpace::Init28);
    Android::RegisterSdkListener(Android::Q, art::gc::space::LargeObjectMapSpace::Init29);
    Android::RegisterSdkListener(Android::R, art::gc::space::LargeObjectMapSpace::Init30);

    Android::RegisterSdkListener(Android::M, art::gc::space::FreeListSpace::Init23);
    Android::RegisterSdkListener(Android::O, art::gc::space::FreeListSpace::Init26);
    Android::RegisterSdkListener(Android::Q, art::gc::space::FreeListSpace::Init29);
    Android::RegisterSdkListener(Android::R, art::gc::space::FreeListSpace::Init30);
}

void LargeObjectSpace::Init23() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectSpace_offset__ = {
            .num_bytes_allocated_ = 64,
            .num_objects_allocated_ = 72,
            .total_bytes_allocated_ = 80,
            .total_objects_allocated_ = 88,
            .begin_ = 96,
            .end_ = 104,
        };

        __LargeObjectSpace_size__ = {
            .THIS = 112,
        };
    } else {
        __LargeObjectSpace_offset__ = {
            .num_bytes_allocated_ = 32,
            .num_objects_allocated_ = 40,
            .total_bytes_allocated_ = 48,
            .total_objects_allocated_ = 56,
            .begin_ = 64,
            .end_ = 68,
        };

        __LargeObjectSpace_size__ = {
            .THIS = 72,
        };
    }
}

void LargeObjectSpace::Init29() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectSpace_offset__ = {
            .num_bytes_allocated_ = 104,
            .num_objects_allocated_ = 112,
            .total_bytes_allocated_ = 120,
            .total_objects_allocated_ = 128,
            .begin_ = 136,
            .end_ = 144,
        };

        __LargeObjectSpace_size__ = {
            .THIS = 152,
        };
    } else {
        __LargeObjectSpace_offset__ = {
            .num_bytes_allocated_ = 64,
            .num_objects_allocated_ = 72,
            .total_bytes_allocated_ = 80,
            .total_objects_allocated_ = 88,
            .begin_ = 96,
            .end_ = 100,
        };

        __LargeObjectSpace_size__ = {
            .THIS = 104,
        };
    }
}

void LargeObjectSpace::Init30() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectSpace_offset__ = {
            .lock_ = 304,
            .num_bytes_allocated_ = 344,
            .num_objects_allocated_ = 352,
            .total_bytes_allocated_ = 360,
            .total_objects_allocated_ = 368,
            .begin_ = 376,
            .end_ = 384,
        };

        __LargeObjectSpace_size__ = {
            .THIS = 392,
        };
    } else {
        __LargeObjectSpace_offset__ = {
            .lock_ = 160,
            .num_bytes_allocated_ = 192,
            .num_objects_allocated_ = 200,
            .total_bytes_allocated_ = 208,
            .total_objects_allocated_ = 216,
            .begin_ = 224,
            .end_ = 228,
        };

        __LargeObjectSpace_size__ = {
            .THIS = 232,
        };
    }
}

void LargeObjectMapSpace::Init23() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 168,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 192,
        };
    } else {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 112,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 128,
        };

        /* 6.0.1 */
        if (Android::Patch() >= 1) {
            __LargeObjectMapSpace_offset__.large_objects_ = 120;
            __LargeObjectMapSpace_size__.THIS = 136;
        }
    }
}

void LargeObjectMapSpace::Init24() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 168,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 192,
        };
    } else {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 120,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 136,
        };
    }
}

void LargeObjectMapSpace::Init26() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 176,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 200,
        };
    } else {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 112,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 124,
        };
    }
}

void LargeObjectMapSpace::Init28() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 168,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 192,
        };
    } else {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 112,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 124,
        };
    }
}

void LargeObjectMapSpace::Init29() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 152,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 176,
        };
    } else {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 104,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 116,
        };
    }
}

void LargeObjectMapSpace::Init30() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 392,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 416,
        };
    } else {
        __LargeObjectMapSpace_offset__ = {
            .large_objects_ = 232,
        };

        __LargeObjectMapSpace_size__ = {
            .THIS = 244,
        };
    }
}

void LargeObjectMapSpace::LargeObject::Init() {
    if (CoreApi::Bits() == 64) {
        __LargeObject_offset__ = {
            .mem_map = 0,
            .is_zygote = 72,
        };
    } else {
        __LargeObject_offset__ = {
            .mem_map = 0,
            .is_zygote = 40,
        };
    }
}

void LargeObjectMapSpace::LargeObjectsPair::Init() {
    if (CoreApi::Bits() == 64) {
        __LargeObjectsPair_offset__ = {
            .first = 0,
            .second = 8,
        };
    } else {
        __LargeObjectsPair_offset__ = {
            .first = 0,
            .second = 4,
        };
    }
}

void AllocationInfo::Init() {
    __AllocationInfo_offset__ = {
        .prev_free_ = 0,
        .alloc_size_ = 4,
    };

    __AllocationInfo_size__ = {
        .THIS = 8,
    };
}

void FreeListSpace::Init23() {
    if (CoreApi::Bits() == 64) {
        __FreeListSpace_offset__ = {
            .mem_map_ = 112,
            .allocation_info_map_ = 120,
            .allocation_info_ = 128,
            .free_end_ = 176,
            .free_blocks_ = 184,
        };

        __FreeListSpace_size__ = {
            .THIS = 208,
        };
    } else {
        __FreeListSpace_offset__ = {
            .mem_map_ = 72,
            .allocation_info_map_ = 76,
            .allocation_info_ = 80,
            .free_end_ = 136,
            .free_blocks_ = 140,
        };

        __FreeListSpace_size__ = {
            .THIS = 152,
        };
    }
}

void FreeListSpace::Init26() {
    if (CoreApi::Bits() == 64) {
        __FreeListSpace_offset__ = {
            .mem_map_ = 112,
            .allocation_info_map_ = 120,
            .allocation_info_ = 128,
            .free_end_ = 176,
            .free_blocks_ = 184,
        };

        __FreeListSpace_size__ = {
            .THIS = 208,
        };
    } else {
        __FreeListSpace_offset__ = {
            .mem_map_ = 72,
            .allocation_info_map_ = 76,
            .allocation_info_ = 80,
            .free_end_ = 116,
            .free_blocks_ = 120,
        };

        __FreeListSpace_size__ = {
            .THIS = 132,
        };
    }
}

void FreeListSpace::Init29() {
    if (CoreApi::Bits() == 64) {
        __FreeListSpace_offset__ = {
            .mem_map_ = 152,
            .allocation_info_map_ = 224,
            .allocation_info_ = 296,
            .free_end_ = 304,
            .free_blocks_ = 312,
        };

        __FreeListSpace_size__ = {
            .THIS = 336,
        };
    } else {
        __FreeListSpace_offset__ = {
            .mem_map_ = 104,
            .allocation_info_map_ = 144,
            .allocation_info_ = 184,
            .free_end_ = 188,
            .free_blocks_ = 192,
        };

        __FreeListSpace_size__ = {
            .THIS = 204,
        };
    }
}

void FreeListSpace::Init30() {
    if (CoreApi::Bits() == 64) {
        __FreeListSpace_offset__ = {
            .mem_map_ = 392,
            .allocation_info_map_ = 464,
            .allocation_info_ = 536,
            .free_end_ = 544,
            .free_blocks_ = 552,
        };

        __FreeListSpace_size__ = {
            .THIS = 576,
        };
    } else {
        __FreeListSpace_offset__ = {
            .mem_map_ = 232,
            .allocation_info_map_ = 272,
            .allocation_info_ = 312,
            .free_end_ = 316,
            .free_blocks_ = 320,
        };

        __FreeListSpace_size__ = {
            .THIS = 332,
        };
    }
}

bool LargeObjectSpace::IsFreeListSpace() {
    return !strcmp(GetName(), FREELIST_SPACE);
};

bool LargeObjectSpace::IsMemMapSpace() {
    return !strcmp(GetName(), MEMMAP_SPACE);
}

bool LargeObjectMapSpace::IsVaildSpace() {
    cxx::map& large_objects_ = GetLargeObjectsCache();
    return large_objects_.Ptr() &&  CoreApi::IsVirtualValid(large_objects_.Ptr());
}

void LargeObjectMapSpace::Walk(std::function<bool (mirror::Object& object)> visitor, bool check) {
    cxx::map& large_objects_ = GetLargeObjectsCache();
    for (const auto& value : large_objects_) {
        LargeObjectMapSpace::LargeObjectsPair pair = value;
        api::MemoryRef ref = pair.first();
        if (ref.IsValid()) {
            mirror::Object object(ref);
            if (object.IsValid()) {
                visitor(object);
            } else if (check) {
                LOGE("0x%" PRIx64 " is bad object on %s!!\n", object.Ptr(), GetName());
            }
        }
    }
}

cxx::map& LargeObjectMapSpace::GetLargeObjectsCache() {
    if (!large_objects_cache.Ptr()) {
        large_objects_cache = large_objects();
        large_objects_cache.copyRef(this);
        large_objects_cache.Prepare(false);

#if defined(__ART_LARGE_OBJECT_MAP_SPACE_PARSER__)
#endif // __ART_LARGE_OBJECT_MAP_SPACE_PARSER__
    }
    return large_objects_cache;
}

uint64_t FreeListSpace::GetSlotIndexForAddress(uint64_t address) {
    return (address - begin()) / kLargeObjectAlignment;
}

uint64_t FreeListSpace::GetAllocationInfoForAddress(uint64_t address) {
    return GetAlloctionInfoCache().Ptr() + (GetSlotIndexForAddress(address) * SIZEOF(AllocationInfo));
}

uint64_t FreeListSpace::GetAddressForAllocationInfo(AllocationInfo& info) {
    return GetAllocationAddressForSlot((info.Ptr() - GetAlloctionInfoCache().Ptr()) / SIZEOF(AllocationInfo));
}

uint64_t FreeListSpace::GetAllocationAddressForSlot(uint64_t slot) {
    return begin() + (slot * kLargeObjectAlignment);
}

void FreeListSpace::Walk(std::function<bool (mirror::Object& object)> visitor, bool check) {
    uint64_t free_end_start = end() - free_end();
    api::MemoryRef block_cache = begin();
    block_cache.Prepare(false);

    AllocationInfo cur_info = GetAlloctionInfoCache();
    AllocationInfo end_info = GetAllocationInfoForAddress(free_end_start);

    while (cur_info.Ptr() < end_info.Ptr()) {
        if (!cur_info.IsFree()) {
            uint64_t byte_start = GetAddressForAllocationInfo(cur_info);
            mirror::Object object(byte_start, block_cache);
            if (object.IsValid()) {
                visitor(object);
            } else if (check) {
                LOGE("0x%" PRIx64 " is bad object on %s!!\n", object.Ptr(), GetName());
            }
        }
        cur_info.MoveNexInfo();
    }
}

api::MemoryRef& FreeListSpace::GetAlloctionInfoCache() {
    if (!allocation_info_cache.Ptr()) {
        allocation_info_cache = allocation_info();
        allocation_info_cache.Prepare(false);
    }
    return allocation_info_cache;
}

bool FreeListSpace::IsVaildSpace() {
    return begin() && CoreApi::IsVirtualValid(begin());
}

} // namespace space
} // namespace gc
} // namespace art
