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
#include "cxx/split_buffer.h"

struct cxx_split_buffer_OffsetTable __cxx_split_buffer_offset__;
struct cxx_split_buffer_SizeTable __cxx_split_buffer_size__;

namespace cxx {

void split_buffer::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __cxx_split_buffer_offset__ = {
        .__first_ = 0,
        .__begin_ = 8 / cap,
        .__end_ = 16 / cap,
        .__end_cap_ = 24 / cap,
    };

    __cxx_split_buffer_size__ = {
        .THIS = 32 / cap,
    };
}

} // namespace cxx
