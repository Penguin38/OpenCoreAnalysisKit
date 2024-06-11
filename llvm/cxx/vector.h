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

#ifndef LLVM_CXX_VECTOR_H_
#define LLVM_CXX_VECTOR_H_

#include "api/memory_ref.h"

struct cxx_vector_OffsetTable {
    uint32_t __begin_;
    uint32_t __end_;
    uint32_t __value_;
};

struct cxx_vector_SizeTable {
    uint32_t THIS;
};

extern struct cxx_vector_OffsetTable __cxx_vector_offset__;
extern struct cxx_vector_SizeTable __cxx_vector_size__;

namespace cxx {

class vector : public api::MemoryRef {
public:
    vector() {}
    vector(uint64_t v) : api::MemoryRef(v) {}
    vector(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    vector(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    vector(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(vector& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(vector& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __begin() { return VALUEOF(cxx_vector, __begin_); }
    inline uint64_t __end() { return VALUEOF(cxx_vector, __end_); }
    inline uint64_t __value() { return VALUEOF(cxx_vector, __value_); }

    inline void SetEntrySize(uint64_t s) { entry_size = s; }

    class iterator {
    public:
        iterator(uint64_t index, uint64_t size)
            : current(index), entry_size(size) {}
        iterator operator++();
        bool operator==(iterator other) const;
        bool operator!=(iterator other) const;
        uint64_t operator*();
    private:
        uint64_t current;
        uint64_t entry_size;
    };

    iterator begin();
    iterator end();
    uint64_t size();

private:
    uint64_t entry_size;
};

} // namespace cxx

#endif  // LLVM_CXX_VECTOR_H_
