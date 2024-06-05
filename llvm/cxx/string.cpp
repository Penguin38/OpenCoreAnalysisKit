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
#include "cxx/string.h"

struct cxx_string_OffsetTable __cxx_string_offset__;
struct cxx_string_SizeTable __cxx_string_size__;

namespace cxx {

void string::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __cxx_string_offset__ = {
        .__l__cap_ = 0,
        .__l__size_ = 8 / cap,
        .__l__data_ = 16 / cap,
        .__s__size_ = 0,
        .__s__data_ = 1,
    };

    __cxx_string_size__ = {
        .THIS = 24 / cap,
    };
}

const char* string::c_str() {
    if (!(__s__size() & __short_mask)) {
        return reinterpret_cast<const char *>(__s__data());
    } else {
        api::MemoryRef ref(__l__data(), this);
        return reinterpret_cast<const char *>(ref.Real());
    }
}

} // namespace cxx

