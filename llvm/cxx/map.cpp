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
#include "cxx/map.h"

struct cxx_map_OffsetTable __cxx_map_offset__;
struct cxx_map_SizeTable __cxx_map_size__;
struct cxx_map_pair_OffsetTable __cxx_map_pair_offset__;
struct cxx_map_pair_SizeTable __cxx_map_pair_size__;

namespace cxx {

void map::Init() {
    uint32_t cap = 64 / CoreApi::GetPointSize();
    __cxx_map_offset__ = {
        .__begin_node_ = 0,
        .__pair1_ = 8 / cap,
        .__pair3_ = 16 / cap,
    };

    __cxx_map_size__ = {
        .THIS = 24 / cap,
    };
}

void map::pair::Init() {
    uint32_t cap = 64 / CoreApi::GetPointSize();
    __cxx_map_pair_offset__ = {
        .__left_ = 0,
        .__right_ = 8 / cap,
        .__parent_ = 16 / cap,
        .__color_ = 24 / cap,
        .__data_ = 32 / cap,
    };

    __cxx_map_pair_size__ = {
        .THIS = 32 / cap,
    };
}

map::iterator map::begin() {
    map::iterator it(this, 0);
    if (!cache.size()) {
        map::pair __pair1_ = __pair1();
        it.traversal(__pair1_);
    }
    return it;
}

map::iterator map::end() {
    map::iterator it(this, size());
    return it;
}

uint64_t map::size() {
    return __pair3();
}

void map::iterator::traversal(pair& p) {
    if (p.__left()) {
        map::pair left(p.__left(), p);
        traversal(left);
    }

    if (p.__right()) {
        map::pair right(p.__right(), p);
        traversal(right);
    }

    __map_->cache.push_back(p);
}

map::iterator& map::iterator::operator++() {
    current++;
    return *this;
}

bool map::iterator::operator==(iterator other) const {
    return current == other.current;
}

bool map::iterator::operator!=(iterator other) const {
    return current != other.current;
}

uint64_t map::iterator::operator*() {
    pair& p1 = __map_->cache[current];
    return p1.__data();
}

} // namespace cxx

