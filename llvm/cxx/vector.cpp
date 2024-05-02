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
#include "cxx/vector.h"

struct cxx_vector_OffsetTable __cxx_vector_offset__;
struct cxx_vector_SizeTable __cxx_vector_size__;

namespace cxx {

void vector::Init() {
    uint32_t cap = 64 / CoreApi::GetPointSize();
    __cxx_vector_offset__ = {
        .__begin_ = 0,
        .__end_ = 8 / cap,
        .__value_ = 16 / cap,
    };

    __cxx_vector_size__ = {
        .THIS = 24 / cap,
    };
}

vector::iterator vector::begin() {
    return vector::iterator(__begin(), entry_size);
}

vector::iterator vector::end() {
    return vector::iterator(__end(), entry_size);
}

uint64_t vector::size() {
    return (__end() - __begin()) / entry_size;
}

vector::iterator vector::iterator::operator++() {
    current += entry_size;
    return *this;
}

bool vector::iterator::operator==(iterator other) const {
    return current == other.current;
}

bool vector::iterator::operator!=(iterator other) const {
    return current != other.current;
}

uint64_t vector::iterator::operator*() {
    return current;
}

} // namespace cxx

