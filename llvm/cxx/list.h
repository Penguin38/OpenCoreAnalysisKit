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

#ifndef LLVM_CXX_LIST_H_
#define LLVM_CXX_LIST_H_

#include "api/memory_ref.h"

struct cxx_list_OffsetTable {
    uint32_t __prev_;
    uint32_t __next_;
    uint32_t __value_;
};

struct cxx_list_SizeTable {
    uint32_t THIS;
};

extern struct cxx_list_OffsetTable __cxx_list_offset__;
extern struct cxx_list_SizeTable __cxx_list_size__;

namespace cxx {

class list : public api::MemoryRef {
public:
    list() {}
    list(uint64_t v) : api::MemoryRef(v) {}
    list(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    list(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    list(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> list(U *v) : api::MemoryRef(v) {}
    template<typename U> list(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(list& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(list& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __prev() { return VALUEOF(cxx_list, __prev_); }
    inline uint64_t __next() { return VALUEOF(cxx_list, __next_); }
    inline uint64_t __value() { return VALUEOF(cxx_list, __value_); }

    class iterator {
    public:
        iterator(uint64_t index) {
            current = index;
            current.Prepare(false);
        }
        iterator& operator++();
        bool operator==(iterator other) const;
        bool operator!=(iterator other) const;
        uint64_t operator*();
    private:
        api::MemoryRef current;
    };

    iterator begin();
    iterator end();
    uint64_t size();
};

} // namespace cxx

#endif  // LLVM_CXX_LIST_H_
