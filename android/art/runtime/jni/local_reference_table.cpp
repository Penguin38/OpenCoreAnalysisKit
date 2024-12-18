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

#include "logger/log.h"
#include "api/core.h"
#include "android.h"
#include "runtime/jni/local_reference_table.h"

struct LocalReferenceTable_OffsetTable __LocalReferenceTable_offset__;
struct LrtEntry_OffsetTable __LrtEntry_offset__;
struct LrtEntry_SizeTable __LrtEntry_size__;

namespace art {
namespace jni {

void LocalReferenceTable::Init() {
    if (CoreApi::Bits() == 64) {
        __LocalReferenceTable_offset__ = {
            .segment_state_ = 4,
            .tables_ = 24,
        };
    } else {
        __LocalReferenceTable_offset__ = {
            .segment_state_ = 4,
            .tables_ = 20,
        };
    }

    LrtEntry::Init();
}

void LrtEntry::Init() {
    __LrtEntry_offset__.root_ = 0;
    __LrtEntry_size__.THIS = 4;
}

void LocalReferenceTable::Walk(std::function<bool (mirror::Object& object)> fn) {
    auto callback = [&](art::mirror::Object& object, uint64_t idx) -> bool {
        fn(object);
        return false;
    };
    Walk(callback);
}

void LocalReferenceTable::Walk(std::function<bool (mirror::Object& object, uint64_t idx)> fn) {
    mirror::Object object = 0x0;
    uint32_t top_index_ = segment_state();

    for (int idx = 0; idx < top_index_; ++idx) {
        // FOR TEST
        if (object.IsValid()) fn(object, idx);
    }
}

} // namespace jni
} // namespace art
