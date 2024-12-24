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
#include "runtime/gc/space/bump_pointer_space.h"
#include "runtime/runtime_globals.h"
#include "cxx/vector.h"

struct BumpPointerSpace_OffsetTable __BumpPointerSpace_offset__;

namespace art {
namespace gc {
namespace space {

void BumpPointerSpace::Init() {
    Android::RegisterSdkListener(Android::M, art::gc::space::BumpPointerSpace::Init26);
    Android::RegisterSdkListener(Android::N, art::gc::space::BumpPointerSpace::Init26);
    Android::RegisterSdkListener(Android::O, art::gc::space::BumpPointerSpace::Init26);
    Android::RegisterSdkListener(Android::U, art::gc::space::BumpPointerSpace::Init34);
}

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

cxx::deque& BumpPointerSpace::GetBlockSizesCache() {
    if (!block_sizes_cache.Ptr()) {
        block_sizes_cache = block_sizes();
        block_sizes_cache.copyRef(this);
        block_sizes_cache.SetBlockSize(CoreApi::GetPointSize());
    }
    return block_sizes_cache;
}

std::deque<uint64_t>& BumpPointerSpace::GetBlockSizes() {
    if (block_sizes_second_cache.empty()) {
        cxx::deque& block_sizes_ = GetBlockSizesCache();
        if (block_sizes_.size()) {
            for (auto& value : block_sizes_) {
                api::MemoryRef& block_size = value;
                block_sizes_second_cache.push_back(block_size.valueOf());
            }
        }
    }
    return block_sizes_second_cache;
}

void BumpPointerSpace::SlowWalk(std::function<bool (mirror::Object& object)> visitor) {
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
            pos = object.NextValidOffset(end);
        }
    }
}

void BumpPointerSpace::Walk(std::function<bool (mirror::Object& object)> visitor, bool check) {
    if (Android::Sdk() < Android::V) {
        SlowWalk(visitor);
        return;
    }

    uint64_t pos = Begin();
    uint64_t end = End();
    uint64_t main_end = pos;

    mirror::Object object_cache = pos;
    object_cache.Prepare(false);

    uint64_t main_block_size_tmp = main_block_size();
    std::deque<uint64_t>& block_sizes_ = GetBlockSizes();
    if (!block_sizes_.size()) {
        main_block_size_tmp = end - pos;
    }

    main_end = Begin() + main_block_size_tmp;
    if (!block_sizes_.size()) {
        end = main_end;
    } else {
        // do nothing
    }

    while (pos < main_end) {
        mirror::Object object(pos, object_cache);
        if (object.IsNonLargeValid()) {
            visitor(object);
            pos = GetNextObject(object);
        } else {
            pos = object.NextValidOffset(main_end);
            if (check && pos < main_end) LOGE("Region:[0x%" PRIx64 ", 0x%" PRIx64 ") %s has bad object!!\n", object.Ptr(), pos, GetName());
        }
    }

    pos = main_end;

    if (block_sizes_.size()) {
        for (const auto& block_size : block_sizes_) {
            uint64_t cur_pos = pos;
            uint64_t cur_end = pos + block_size;

            while (cur_pos < cur_end) {
                mirror::Object object(cur_pos, object_cache);
                if (object.IsNonLargeValid()) {
                    visitor(object);
                    cur_pos = GetNextObject(object);
                } else {
                    cur_pos = object.NextValidOffset(cur_end);
                    if (check && cur_pos < cur_end) LOGE("Region:[0x%" PRIx64 ", 0x%" PRIx64 ") %s has bad object!!\n", object.Ptr(), cur_pos, GetName());
                }
            }
            pos += block_size;
        }
    }
}

} // namespace space
} // namespace gc
} // namespace art
