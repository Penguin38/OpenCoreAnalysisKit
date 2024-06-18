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

#ifndef LLVM_CXX_DEQUE_H_
#define LLVM_CXX_DEQUE_H_

#include "api/memory_ref.h"
#include "cxx/split_buffer.h"

struct cxx_deque_OffsetTable {
    uint32_t __map_;
    uint32_t __start_;
    uint32_t __size_;
};

struct cxx_deque_SizeTable {
    uint32_t THIS;
};

extern struct cxx_deque_OffsetTable __cxx_deque_offset__;
extern struct cxx_deque_SizeTable __cxx_deque_size__;

namespace cxx {

class deque : public api::MemoryRef {
public:
    deque() : api::MemoryRef() {}
    deque(uint64_t v) : api::MemoryRef(v) {}
    deque(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    deque(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    deque(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(deque& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(deque& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __map() { return Ptr() + OFFSET(cxx_deque, __map_); }
    inline uint64_t __start() { return VALUEOF(cxx_deque, __start_); }
    inline uint64_t __size() { return VALUEOF(cxx_deque, __size_); }

    uint64_t size();
    split_buffer& Map();
    uint64_t begin();
    uint64_t end();
private:
    split_buffer __map_cache = 0x0;
};

} // namespace cxx

#endif  // LLVM_CXX_DEQUE_H_
