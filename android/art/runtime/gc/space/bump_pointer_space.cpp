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
#include "cxx/vector.h"

struct BumpPointerSpace_OffsetTable __BumpPointerSpace_offset__;

namespace art {
namespace gc {
namespace space {

void BumpPointerSpace::Init26() {
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

cxx::deque& BumpPointerSpace::GetBlockSizes() {
    if (!block_sizes_cache.Ptr()) {
        block_sizes_cache = block_sizes();
        block_sizes_cache.copyRef(this);
    }
    return block_sizes_cache;
}

#if 1
void BumpPointerSpace::Walk(std::function<bool (mirror::Object& object)> visitor) {
    uint64_t pos = Begin();
    uint64_t end = End();

    mirror::Object object_cache = pos;
    object_cache.Prepare(false);

    // slow walk
    while (pos < end) {
        mirror::Object object(pos, object_cache);
        if (object.IsValid()) {
            visitor(object);
            pos = GetNextObject(object);
        } else {
            pos += kObjectAlignment;
        }
    }
}
#else
void BumpPointerSpace::Walk(std::function<bool (mirror::Object& object)> visitor) {
    uint64_t pos = Begin();
    uint64_t end = End();
    uint64_t main_end = pos;

    mirror::Object object_cache = pos;
    object_cache.Prepare(false);

    uint64_t main_block_size_tmp = main_block_size();
    if (!GetBlockSizes().size()) {
        main_block_size_tmp = end - pos;
    }

    main_end = Begin() + main_block_size_tmp;
    if (!GetBlockSizes().size()) {
        end = main_end;
    } else {
        // do nothing
    }

    while (pos < main_end) {
        mirror::Object object(pos, object_cache);
        if (object.IsValid()) {
            visitor(object);
            pos = GetNextObject(object);
        } else {
            pos += kObjectAlignment;
        }
    }

    pos = main_end;

    if (GetBlockSizes().size()) {
        cxx::vector block_sizes_copy = GetBlockSizes().Ptr() + OFFSET(cxx_split_buffer, __begin_);
        block_sizes_copy.copyRef(GetBlockSizes());
        block_sizes_copy.SetEntrySize(CoreApi::GetPointSize());

        for (const auto& value : block_sizes_copy) {
            api::MemoryRef ref = value;
            api::MemoryRef block_size = ref.valueOf();
            mirror::Object obj(pos, object_cache);
            mirror::Object end_obj(pos + block_size.valueOf(), object_cache);

            while (obj.Ptr() < end_obj.Ptr() && obj.IsValid()) {
                visitor(obj);
                obj = GetNextObject(obj);
                obj.copyRef(object_cache);
            }
            pos += block_size.valueOf();
        }
    }
}
#endif

} // namespace space
} // namespace gc
} // namespace art
