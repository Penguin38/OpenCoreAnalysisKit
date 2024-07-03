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
#include "cxx/list.h"

struct cxx_list_OffsetTable __cxx_list_offset__;
struct cxx_list_SizeTable __cxx_list_size__;

namespace cxx {

void list::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __cxx_list_offset__ = {
        .__prev_ = 0,
        .__next_ = 8 / cap,
        .__value_ = 16 / cap,
    };

    __cxx_list_size__ = {
        .THIS = 24 / cap,
    };
}

list::iterator list::begin() {
    return list::iterator(__next() & CoreApi::GetVabitsMask());
}

list::iterator list::end() {
    return list::iterator(Ptr() & CoreApi::GetVabitsMask());
}

uint64_t list::size() {
    return __value();
}

list::iterator& list::iterator::operator++() {
    list cl = current;
    current = cl.__next() & CoreApi::GetVabitsMask();
    return *this;
}

bool list::iterator::operator==(iterator other) const {
    return current == other.current;
}

bool list::iterator::operator!=(iterator other) const {
    return current != other.current;
}

uint64_t list::iterator::operator*() {
    return current.Ptr() + OFFSET(cxx_list, __value_);
}

} // namespace cxx
