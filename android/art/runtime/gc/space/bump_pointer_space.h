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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_H_

#include "runtime/gc/space/space.h"
#include "cxx/deque.h"
#include <deque>
#include <functional>

struct BumpPointerSpace_OffsetTable {
    uint32_t main_block_size_;
    uint32_t num_blocks_;
    uint32_t block_sizes_;
};

extern struct BumpPointerSpace_OffsetTable __BumpPointerSpace_offset__;

namespace art {
namespace gc {
namespace space {

class BumpPointerSpace : public ContinuousMemMapAllocSpace {
public:
    BumpPointerSpace() : ContinuousMemMapAllocSpace() {}
    BumpPointerSpace(uint64_t v) : ContinuousMemMapAllocSpace(v) {}
    BumpPointerSpace(uint64_t v, LoadBlock* b) : ContinuousMemMapAllocSpace(v, b) {}
    BumpPointerSpace(const ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(ref) {}
    BumpPointerSpace(uint64_t v, ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(v, ref) {}
    BumpPointerSpace(uint64_t v, ContinuousMemMapAllocSpace* ref) : ContinuousMemMapAllocSpace(v, ref) {}

    static void Init26();
    static void Init34();
    inline uint64_t main_block_size() { return VALUEOF(BumpPointerSpace, main_block_size_); }
    inline uint64_t num_blocks() { return VALUEOF(BumpPointerSpace, num_blocks_); }
    inline uint64_t block_sizes() { return Ptr() + OFFSET(BumpPointerSpace, block_sizes_); }

    SpaceType GetType() { return kSpaceTypeBumpPointerSpace; }
    void Walk(std::function<bool (mirror::Object& object)> fn, bool check);
    void SlowWalk(std::function<bool (mirror::Object& object)> fn);

    cxx::deque& GetBlockSizesCache();
    std::deque<uint64_t>& GetBlockSizes();
private:
    // quick memoryref cache
    cxx::deque block_sizes_cache = 0x0;

    // second cache
    std::deque<uint64_t> block_sizes_second_cache;
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_BUMP_POINTER_SPACE_H_
