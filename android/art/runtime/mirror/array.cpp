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

#include "runtime/mirror/array.h"
#include "runtime/mirror/class.h"
#include "common/bit.h"

struct Array_OffsetTable __Array_offset__;
struct Array_SizeTable __Array_size__;

namespace art {
namespace mirror {

void Array::Init() {
    __Array_offset__ = {
        .length_ = 8,
        .first_element_ = 12,
    };

    __Array_size__ = {
        .THIS = 12,
        .length_ = 4,
    };
}

int32_t Array::GetLength() {
    return length();   
}

void* Array::GetRawData(size_t component_size, int32_t index) {
    uint32_t data = Ptr() + RoundUp(OFFSET(Array, first_element_), component_size) + (index * component_size);
    return reinterpret_cast<void*>(data);
}

uint64_t Array::SizeOf() {
    uint64_t component_size_shift = GetClass().GetComponentSizeShift();
    int32_t component_count = GetLength();
    size_t header_size = RoundUp(0xC, 1U << component_size_shift);
    size_t data_size = component_count << component_size_shift;
    return header_size + data_size;
}

} // namespace mirror
} // namespace art
