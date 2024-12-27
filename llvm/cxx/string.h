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

#ifndef LLVM_CXX_STRING_H_
#define LLVM_CXX_STRING_H_

#include "api/memory_ref.h"
#include <string>

struct cxx_string_OffsetTable {
    uint32_t __l__cap_;
    uint32_t __l__size_;
    uint32_t __l__data_;
    uint32_t __s__size_;
    uint32_t __s__data_;
};

struct cxx_string_SizeTable {
    uint32_t THIS;
};

extern struct cxx_string_OffsetTable __cxx_string_offset__;
extern struct cxx_string_SizeTable __cxx_string_size__;

namespace cxx {

class string : public api::MemoryRef {
public:
    static constexpr uint64_t __short_mask = 0x1;
    static constexpr uint64_t __long_mask = 0x1ul;

    string(uint64_t v) : api::MemoryRef(v) {}
    string(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    string(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    string(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(string& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(string& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __s__size() { return VALUEOF(cxx_string, __s__size_); }
    inline uint64_t __s__data() { return Real() + OFFSET(cxx_string, __s__data_); }
    inline uint64_t __l__data() { return VALUEOF(cxx_string, __l__data_); }

    const char *c_str();
    std::string str();
};

} // namespace cxx

#endif  // LLVM_CXX_STRING_H_
