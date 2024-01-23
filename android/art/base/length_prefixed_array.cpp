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

#include "base/length_prefixed_array.h"

struct LengthPrefixedArray_OffsetTable __LengthPrefixedArray_offset__;
struct LengthPrefixedArray_SizeTable __LengthPrefixedArray_size__;

namespace art {

void LengthPrefixedArray::Init() {
    __LengthPrefixedArray_offset__ = {
        .size_ = 0,
        .data_ = 4,
    };

    __LengthPrefixedArray_size__ = {
        .THIS = 4,
    };
}

} // namespace art
