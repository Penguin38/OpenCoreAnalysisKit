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
#include "runtime/gc/space/bump_pointer_space.h"
#include "runtime/runtime_globals.h"

struct BumpPointerSpace_OffsetTable __BumpPointerSpace_offset__;

namespace art {
namespace gc {
namespace space {

void BumpPointerSpace::Init() {
    if (CoreApi::Bits() == 64) {
        __BumpPointerSpace_offset__ = {
            .main_block_size_ = 584,
            .num_blocks_ = 592,
        };
    } else {
        __BumpPointerSpace_offset__ = {
            .main_block_size_ = 328,
            .num_blocks_ = 332,
        };
    }
}

void BumpPointerSpace::Init34() {
    if (CoreApi::Bits() == 64) {
        __BumpPointerSpace_offset__ = {
            .main_block_size_ = 584,
            .block_sizes_ = 592,
        };
    } else {
        __BumpPointerSpace_offset__ = {
            .main_block_size_ = 328,
            .block_sizes_ = 332,
        };
    }
}

void BumpPointerSpace::Walk(std::function<bool (mirror::Object& object)> visitor) {
    uint64_t pos = Begin();
    uint64_t top = End();
    mirror::Object object_cache = pos;
    object_cache.Prepare(false);

    while (pos < top) {
        mirror::Object object(pos, object_cache);
        if (object.IsValid()) {
            visitor(object);
            pos = GetNextObject(object);
        } else {
            pos += kObjectAlignment;
        }
    }
}

} // namespace space
} // namespace gc
} // namespace art
