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

#include "api/core.h"
#include "runtime/managed_stack.h"

struct ManagedStack_OffsetTable __ManagedStack_offset__;
struct ManagedStack_SizeTable __ManagedStack_size__;

namespace art {

void ManagedStack::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __ManagedStack_offset__ = {
        .tagged_top_quick_frame_ = 0,
        .link_ = 8 / cap,
        .top_shadow_frame_ = 16 / cap,
    };

    __ManagedStack_size__ = {
        .THIS = 24 / cap,
    };
}

ManagedStack::TaggedTopQuickFrame& ManagedStack::GetTaggedTopQuickFrame() {
    if (!tagged_top_quick_frame_cache.Ptr()) {
        tagged_top_quick_frame_cache = tagged_top_quick_frame();
        tagged_top_quick_frame_cache.copyRef(this);
    }
    return tagged_top_quick_frame_cache;
}

ShadowFrame& ManagedStack::GetTopShadowFrame() {
    if (!top_shadow_frame_cache.Ptr()) {
        top_shadow_frame_cache = top_shadow_frame();
    }
    return top_shadow_frame_cache;
}

} //namespace art
