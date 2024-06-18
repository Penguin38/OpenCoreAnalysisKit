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

#ifndef LLVM_CXX_SPLIT_BUFFER_H_
#define LLVM_CXX_SPLIT_BUFFER_H_

#include "api/memory_ref.h"

struct cxx_split_buffer_OffsetTable {
    uint32_t __first_;
    uint32_t __begin_;
    uint32_t __end_;
    uint32_t __end_cap_;
};

struct cxx_split_buffer_SizeTable {
    uint32_t THIS;
};

extern struct cxx_split_buffer_OffsetTable __cxx_split_buffer_offset__;
extern struct cxx_split_buffer_SizeTable __cxx_split_buffer_size__;

namespace cxx {

class split_buffer : public api::MemoryRef {
public:
    split_buffer() : api::MemoryRef() {}
    split_buffer(uint64_t v) : api::MemoryRef(v) {}
    split_buffer(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    split_buffer(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    split_buffer(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(split_buffer& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(split_buffer& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __first() { return VALUEOF(cxx_split_buffer, __first_); }
    inline uint64_t __begin() { return VALUEOF(cxx_split_buffer, __begin_); }
    inline uint64_t __end() { return VALUEOF(cxx_split_buffer, __end_); }
    inline uint64_t __end_cap() { return VALUEOF(cxx_split_buffer, __end_cap_); }
};

} // namespace cxx

#endif  // LLVM_CXX_SPLIT_BUFFER_H_
