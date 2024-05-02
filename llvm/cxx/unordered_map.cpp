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
#include "cxx/unordered_map.h"

struct cxx_unordered_map_OffsetTable __cxx_unordered_map_offset__;
struct cxx_unordered_map_SizeTable __cxx_unordered_map_size__;

namespace cxx {

void unordered_map::Init() {
    uint32_t cap = 64 / CoreApi::GetPointSize();
    __cxx_unordered_map_offset__ = {
        .__bucket_list_ = 0,
        .__p1_ = 16 / cap,
        .__p2_ = 24 / cap,
        .__p3_ = 32 / cap,
        .__bucket_list__value_ = 0,
        .__bucket_list__index_ = 8 / cap,
        .__pair__next_ = 0,
        .__pair__hash_ = 8 / cap,
        .__pair__data_ = 16 / cap,
    };

    __cxx_unordered_map_size__ = {
        .THIS = 40 / cap,
        .bucket_list_size = 16 / cap,
        .pair_size = 16 / cap,
    };
}

unordered_map::iterator unordered_map::begin() {
    return unordered_map::iterator(__p1());
}

unordered_map::iterator unordered_map::end() {
    return unordered_map::iterator(0x0);
}

uint64_t unordered_map::size() {
    return __p2();
}

unordered_map::iterator& unordered_map::iterator::operator++() {
    uint64_t next = pair_cache.__next();
    unordered_map::pair tmp = pair_cache;
    pair_cache = next;
    pair_cache.copyRef(tmp);
    return *this;
}

bool unordered_map::iterator::operator==(iterator other) const {
    return pair_cache == other.pair_cache;
}

bool unordered_map::iterator::operator!=(iterator other) const {
    return pair_cache != other.pair_cache;
}

uint64_t unordered_map::iterator::operator*() {
    return pair_cache.__data();
}

} // namespace cxx
