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

#include "runtime/gc/heap.h"

struct Heap_OffsetTable __Heap_offset__;
struct Heap_SizeTable __Heap_size__;

namespace art {
namespace gc {

void Heap::Init() {
    if (CoreApi::GetPointSize() == 64) {
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

std::vector<std::unique_ptr<space::ContinuousSpace>>& Heap::GetContinuousSpaces() {
    if (!continuous_spaces_cache.Ptr()) {
        continuous_spaces_cache = continuous_spaces();
        continuous_spaces_cache.copyRef(this);
        continuous_spaces_cache.SetEntrySize(CoreApi::GetPointSize() / 8);

        for (const auto& value : continuous_spaces_cache) {
            api::MemoryRef ref = value;
            std::unique_ptr<space::ContinuousSpace> space = std::make_unique<space::ContinuousSpace>(ref.valueOf());
            continuous_spaces_second_cache.push_back(std::move(space));
        }
    }
    return continuous_spaces_second_cache;
}

std::vector<std::unique_ptr<space::DiscontinuousSpace>>& Heap::GetDiscontinuousSpaces() {
    if (!discontinuous_spaces_cache.Ptr()) {
        discontinuous_spaces_cache = discontinuous_spaces();
        discontinuous_spaces_cache.copyRef(this);
        discontinuous_spaces_cache.SetEntrySize(CoreApi::GetPointSize() / 8);

        for (const auto& value : discontinuous_spaces_cache) {
            api::MemoryRef ref = value;
            std::unique_ptr<space::DiscontinuousSpace> space = std::make_unique<space::DiscontinuousSpace>(ref.valueOf());
            discontinuous_spaces_second_cache.push_back(std::move(space));
        }
    }
    return discontinuous_spaces_second_cache;
}

} // namespace gc
} // namespace art
