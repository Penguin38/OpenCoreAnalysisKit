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

#include "logger/log.h"
#include "runtime/gc/space/image_space.h"
#include "runtime/runtime_globals.h"
#include "runtime/image.h"

struct ImageSpace_OffsetTable __ImageSpace_offset__;
struct ImageSpace_SizeTable __ImageSpace_size__;

namespace art {
namespace gc {
namespace space {

void ImageSpace::Init() {
    // do nothing
}

void ImageSpace::Walk(std::function<bool (mirror::Object& object)> visitor, bool check) {
    uint64_t pos = Begin() + SIZEOF(ImageHeader);
    uint64_t top = End();
    mirror::Object object_cache = pos;
    object_cache.Prepare(false);

    while (pos < top) {
        mirror::Object object(pos, object_cache);
        if (object.IsNonLargeValid()) {
            visitor(object);
            pos = GetNextObject(object);
        } else {
            pos = object.NextValidOffset(top);
            if (check && pos < top) LOGE("Region:[0x%" PRIx64 ", 0x%" PRIx64 ") %s has bad object!!\n", object.Ptr(), pos, GetName());
        }
    }
}

} // namespace space
} // namespace gc
} // namespace art
