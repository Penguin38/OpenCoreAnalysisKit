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
#include "runtime/gc/space/region_space.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/object.h"
#include "runtime/runtime_globals.h"

struct RegionSpace_OffsetTable __RegionSpace_offset__;
struct RegionSpace_SizeTable __RegionSpace_size__;
struct Region_OffsetTable __Region_offset__;
struct Region_SizeTable __Region_size__;

namespace art {
namespace gc {
namespace space {

void RegionSpace::Init26() {
    if (CoreApi::Bits() == 64) {
        __RegionSpace_offset__ = {
            .num_regions_ = 176,
            .regions_ = 192,
            .mark_bitmap_ = 304,
        };
    } else {
        //TODO
    }
}

void RegionSpace::Init28() {
    if (CoreApi::Bits() == 64) {
        __RegionSpace_offset__ = {
            .num_regions_ = 168,
            .regions_ = 200,
            .mark_bitmap_ = 312,
        };
    } else {
        //TODO
    }
}

void RegionSpace::Init29() {
    if (CoreApi::Bits() == 64) {
        __RegionSpace_offset__ = {
            .num_regions_ = 216,
            .regions_ = 248,
            .mark_bitmap_ = 352,
        };
    } else {
        __RegionSpace_offset__ = {
            .num_regions_ = 128,
            .regions_ = 144,
            .mark_bitmap_ = 200,
        };
    }
}

void RegionSpace::Init30() {
    if (CoreApi::Bits() == 64) {
        __RegionSpace_offset__ = {
            .num_regions_ = 576,
            .regions_ = 608,
            .mark_bitmap_ = 736,
        };
    } else {
        __RegionSpace_offset__ = {
            .num_regions_ = 320,
            .regions_ = 336,
            .mark_bitmap_ = 404,
        };
    }
}

void RegionSpace::Init31() {
    if (CoreApi::Bits() == 64) {
        __RegionSpace_offset__ = {
            .num_regions_ = 576,
            .regions_ = 616,
            .mark_bitmap_ = 744,
        };
    } else {
        __RegionSpace_offset__ = {
            .num_regions_ = 320,
            .regions_ = 348,
            .mark_bitmap_ = 484,
        };
    }
}

void RegionSpace::Region::Init26() {
    if (CoreApi::Bits() == 64) {
        __Region_offset__ = {
            .idx_ = 0,
            .live_bytes_ = 56,
            .begin_ = 8,
            .thread_ = 72,
            .top_ = 16,
            .end_ = 24,
            .objects_allocated_ = 40,
            .alloc_time_ = 48,
            .is_newly_allocated_ = 64,
            .is_a_tlab_ = 65,
            .state_ = 32,
            .type_ = 33,
        };

        __Region_size__ = {
            .THIS = 80,
        };
    } else {
        //TODO
    }
}

void RegionSpace::Region::Init29() {
    if (CoreApi::Bits() == 64) {
        __Region_offset__ = {
            .idx_ = 0,
            .live_bytes_ = 8,
            .begin_ = 16,
            .thread_ = 24,
            .top_ = 32,
            .end_ = 40,
            .objects_allocated_ = 48,
            .alloc_time_ = 56,
            .is_newly_allocated_ = 60,
            .is_a_tlab_ = 61,
            .state_ = 62,
            .type_ = 63,
        };

        __Region_size__ = {
            .THIS = 64,
        };
    } else {
        __Region_offset__ = {
            .idx_ = 0,
            .live_bytes_ = 4,
            .begin_ = 8,
            .thread_ = 12,
            .top_ = 16,
            .end_ = 20,
            .objects_allocated_ = 24,
            .alloc_time_ = 28,
            .is_newly_allocated_ = 32,
            .is_a_tlab_ = 33,
            .state_ = 34,
            .type_ = 35,
        };

        __Region_size__ = {
            .THIS = 36,
        };
    }
}

void RegionSpace::Walk(std::function<bool (mirror::Object& object)> fn) {
    WalkInternal(fn, false);
}

void RegionSpace::WalkInternal(std::function<bool (mirror::Object& object)> visitor, bool only) {
    Region regions_(regions(), this);
    uint64_t num_regions_ = num_regions();
    for (int i = 0; i < num_regions_; ++i) {
        Region r(regions_.Ptr() + i * SIZEOF(Region), regions_);
        uint64_t pos = r.Begin();
        uint64_t top = r.Top();

        if (r.IsFree() || (only && r.IsInToSpace()))
            continue;

        if (r.IsLarge()) {
            mirror::Object object = r.Begin();
            if (object.GetClass().Ptr() != 0x0) {
                visitor(object);
            }
        } else if (r.IsLargeTail()) {
            // Do nothing.
        } else {
            WalkNonLargeRegion(visitor, r);
        }
    }
}

void RegionSpace::WalkNonLargeRegion(std::function<bool (mirror::Object& object)> visitor, RegionSpace::Region& region) {
    uint64_t pos = region.Begin();
    uint64_t begin = pos;
    uint64_t top = region.Top();
    mirror::Object object_cache = pos;
    object_cache.Prepare(false);
    uint64_t bit_mask = CoreApi::GetPointMask();

    const bool need_bitmap =
        region.LiveBytes() != (static_cast<uint64_t>(-1) & bit_mask) &&
        region.LiveBytes() != static_cast<uint64_t>(top - pos);

    if (need_bitmap) {
        GetLiveBitmap().VisitMarkedRange(pos, top, visitor);
    } else {
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
}

accounting::ContinuousSpaceBitmap& RegionSpace::GetLiveBitmap() {
    if (!mark_bitmap_cache.Ptr()) {
        if (Android::Sdk() > Android::Q) {
            mark_bitmap_cache = mark_bitmap();
        } else {
            mark_bitmap_cache = mark_bitmap_v28();
        }
        mark_bitmap_cache.copyRef(this);
        mark_bitmap_cache.Prepare(false);
    }
    return mark_bitmap_cache;
}

} // namespace space
} // namespace gc
} // namespace art
