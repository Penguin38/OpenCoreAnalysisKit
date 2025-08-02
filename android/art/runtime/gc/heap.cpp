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
#include "common/bit.h"
#include "common/exception.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/region_space.h"
#include "runtime/gc/space/image_space.h"
#include "runtime/gc/space/zygote_space.h"
#include "runtime/gc/space/large_object_space.h"
#include "runtime/gc/space/fake_space.h"
#include "runtime/gc/space/bump_pointer_space.h"
#include "runtime/gc/space/rosalloc_space.h"
#include "runtime/gc/space/dlmalloc_space.h"

struct Heap_OffsetTable __Heap_offset__;

namespace art {
namespace gc {

void Heap::Init() {
    Android::RegisterSdkListener(Android::M, art::gc::Heap::Init23);
}

void Heap::Init23() {
    if (CoreApi::Bits() == 64) {
        __Heap_offset__ = {
            .continuous_spaces_ = 0,
            .discontinuous_spaces_ = 24,
        };
    } else {
        __Heap_offset__ = {
            .continuous_spaces_ = 0,
            .discontinuous_spaces_ = 12,
        };
    }
}

cxx::vector& Heap::GetContinuousSpacesCache() {
    if (!continuous_spaces_cache.Ptr()) {
        continuous_spaces_cache = continuous_spaces();
        continuous_spaces_cache.copyRef(this);
        continuous_spaces_cache.SetEntrySize(CoreApi::GetPointSize());

#if defined(__ART_HEAP_CONTINUOUS_SPACE_PARSER__)
        uint64_t point_size = CoreApi::GetPointSize();
        if (!(continuous_spaces_cache.size() >= 2 && continuous_spaces_cache.size() < 64)
                || !!((continuous_spaces_cache.__end() - continuous_spaces_cache.__begin()) % point_size)) {
            LOGD("continuous_spaces_ invalid, do analysis ...\n");
            bool found = false;
            int count = 0;
            uint64_t endloop = RoundUp(continuous_spaces_cache.Ptr(), 0x2000) - SIZEOF(cxx_vector);
            int loopcount = (endloop - continuous_spaces_cache.Ptr()) / point_size;
            do {
                if (continuous_spaces_cache.size() >= 2 && continuous_spaces_cache.size() < 64
                        && !((continuous_spaces_cache.__end() - continuous_spaces_cache.__begin()) % point_size)) {
                    LOGD(">>> 'continuous_spaces_' = 0x%" PRIx64 "\n", continuous_spaces_cache.Ptr());
                    found = true;
                    break;
                }
                count++;
                continuous_spaces_cache.MovePtr(point_size);
            } while(count < loopcount);
        }
#endif
    }
    return continuous_spaces_cache;
}

cxx::vector& Heap::GetDiscontinuousSpacesCache() {
    if (!discontinuous_spaces_cache.Ptr()) {
        discontinuous_spaces_cache = discontinuous_spaces();
        discontinuous_spaces_cache.copyRef(this);
        discontinuous_spaces_cache.SetEntrySize(CoreApi::GetPointSize());

#if defined(__ART_HEAP_DISCONTINUOUS_SPACE_PARSER__)
        uint64_t point_size = CoreApi::GetPointSize();
        if (!(discontinuous_spaces_cache.size() < 8)
                || !!((discontinuous_spaces_cache.__end() - discontinuous_spaces_cache.__begin()) % point_size)) {
            LOGD("discontinuous_spaces_ invalid, do analysis ...\n");
            bool found = false;
            int count = 0;
            uint64_t endloop = RoundUp(discontinuous_spaces_cache.Ptr(), 0x2000) - SIZEOF(cxx_vector);
            int loopcount = (endloop - discontinuous_spaces_cache.Ptr()) / point_size;
            do {
                if (discontinuous_spaces_cache.size() < 8
                        && !((discontinuous_spaces_cache.__end() - discontinuous_spaces_cache.__begin()) % point_size)) {
                    LOGD(">>> 'discontinuous_spaces_' = 0x%" PRIx64 "\n", discontinuous_spaces_cache.Ptr());
                    found = true;
                    break;
                }
                count++;
                discontinuous_spaces_cache.MovePtr(point_size);
            } while(count < loopcount);
        }
#endif
    }
    return discontinuous_spaces_cache;
}

std::vector<std::unique_ptr<space::ContinuousSpace>>& Heap::GetContinuousSpaces() {
    if (!continuous_spaces_second_cache.size()) {
        for (const auto& value : GetContinuousSpacesCache()) {
            api::MemoryRef ref = value;
            std::unique_ptr<space::ContinuousSpace> space = std::make_unique<space::ContinuousSpace>(ref.valueOf());
            if (space->IsRegionSpace()) {
                std::unique_ptr<space::RegionSpace> region_space = std::make_unique<space::RegionSpace>(space->Ptr(), space->Block());
                continuous_spaces_second_cache.push_back(std::move(region_space));
            } else if (space->IsImageSpace()) {
                std::unique_ptr<space::ImageSpace> image_space = std::make_unique<space::ImageSpace>(space->Ptr(), space->Block());
                continuous_spaces_second_cache.push_back(std::move(image_space));
            } else if (space->IsZygoteSpace()) {
                std::unique_ptr<space::ZygoteSpace> zygote_space = std::make_unique<space::ZygoteSpace>(space->Ptr(), space->Block());
                continuous_spaces_second_cache.push_back(std::move(zygote_space));
            } else if (space->IsBumpPointerSpace()) {
                std::unique_ptr<space::BumpPointerSpace> bump_pointer_space = std::make_unique<space::BumpPointerSpace>(space->Ptr(), space->Block());
                continuous_spaces_second_cache.push_back(std::move(bump_pointer_space));
            } else if (space->IsMallocSpace()) {
                if (space->IsRosAllocSpace()) {
                    std::unique_ptr<space::RosAllocSpace> rosalloc_space = std::make_unique<space::RosAllocSpace>(space->Ptr(), space->Block());
                    continuous_spaces_second_cache.push_back(std::move(rosalloc_space));
                } else if (space->IsDlMallocSpace()) {
                    std::unique_ptr<space::DlMallocSpace> dlmalloc_space = std::make_unique<space::DlMallocSpace>(space->Ptr(), space->Block());
                    continuous_spaces_second_cache.push_back(std::move(dlmalloc_space));
                } else {
                    std::unique_ptr<space::MallocSpace> malloc_space = std::make_unique<space::MallocSpace>(space->Ptr(), space->Block());
                    continuous_spaces_second_cache.push_back(std::move(malloc_space));
                }
            } else {
                continuous_spaces_second_cache.push_back(std::move(space));
            }
        }

#if 0
        // fake Object.newInstance feature.
        if (space::FakeSpace::Create()) {
            std::unique_ptr<space::FakeSpace> fake_space = std::make_unique<space::FakeSpace>(space::FakeSpace::FAKE_SPACE_PTR);
            continuous_spaces_second_cache.push_back(std::move(fake_space));
        }
#endif
    }
    return continuous_spaces_second_cache;
}

std::vector<std::unique_ptr<space::DiscontinuousSpace>>& Heap::GetDiscontinuousSpaces() {
    if (!discontinuous_spaces_second_cache.size()) {
        for (const auto& value : GetDiscontinuousSpacesCache()) {
            api::MemoryRef ref = value;
            std::unique_ptr<space::DiscontinuousSpace> space = std::make_unique<space::DiscontinuousSpace>(ref.valueOf());
            if (space->IsLargeObjectSpace()) {
                std::unique_ptr<space::LargeObjectSpace> large_space = std::make_unique<space::LargeObjectSpace>(space->Ptr(), space->Block());
                if (large_space->IsFreeListSpace()) {
                    std::unique_ptr<space::FreeListSpace> freelist_space = std::make_unique<space::FreeListSpace>(large_space->Ptr(), large_space->Block());
                    discontinuous_spaces_second_cache.push_back(std::move(freelist_space));
                } else if (large_space->IsMemMapSpace()) {
                    std::unique_ptr<space::LargeObjectMapSpace> memmap_space = std::make_unique<space::LargeObjectMapSpace>(large_space->Ptr(), large_space->Block());
                    discontinuous_spaces_second_cache.push_back(std::move(memmap_space));
                } else {
                    discontinuous_spaces_second_cache.push_back(std::move(large_space));
                }
            } else {
                discontinuous_spaces_second_cache.push_back(std::move(space));
            }
        }
    }
    return discontinuous_spaces_second_cache;
}

space::ContinuousSpace* Heap::FindContinuousSpaceFromObject(mirror::Object& object) {
    for (const auto& space : GetContinuousSpaces()) {
        if (object.Ptr() >= space->Begin() && object.Ptr() < space->Limit())
            return space.get();
    }
    return nullptr;
}

} // namespace gc
} // namespace art
