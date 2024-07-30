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
#include "cxx/deque.h"

struct cxx_deque_OffsetTable __cxx_deque_offset__;
struct cxx_deque_SizeTable __cxx_deque_size__;

namespace cxx {

void deque::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __cxx_deque_offset__ = {
        .__map_ = 0,
        .__start_ = 32 / cap,
        .__size_ = 40 / cap,
    };

    __cxx_deque_size__ = {
        .THIS = 48 / cap,
    };
}

uint64_t deque::size() {
    return __size();
}

split_buffer& deque::Map() {
    if (!__map_cache.Ptr()) {
        __map_cache = __map();
        __map_cache.copyRef(this);
    }
    return __map_cache;
}

deque::iterator deque::begin() {
    api::MemoryRef __mp = Map().__begin() + (__start() / __block_size()) * pointer_size;
    return deque::iterator(__mp, Map().empty() ? 0 : __mp.valueOf() + (__start() % __block_size()) * pointer_size, pointer_size);
}

deque::iterator deque::end() {
    uint64_t __p = size() + __start();
    api::MemoryRef __mp = Map().__begin() + (__p / __block_size()) * pointer_size;
    return deque::iterator(__mp, Map().empty() ? 0 : __mp.valueOf() + (__p % __block_size()) * pointer_size, pointer_size);
}

deque::iterator& deque::iterator::operator++() {
    pointer.MovePtr(pointer_size);
    if (pointer.Ptr() - map.valueOf() == block_size * pointer_size) {
        map.MovePtr(CoreApi::GetPointSize());
        pointer = map.valueOf();
        pointer.Prepare(false);
    }
    return *this;
}

bool deque::iterator::operator==(deque::iterator other) const {
    return pointer == other.pointer;
}

bool deque::iterator::operator!=(deque::iterator other) const {
    return pointer != other.pointer;
}

api::MemoryRef& deque::iterator::operator*() {
    return pointer;
}

} // namespace cxx
