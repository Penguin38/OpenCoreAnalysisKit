/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef LLVM_CXX_UNORDERED_SET_H_
#define LLVM_CXX_UNORDERED_SET_H_

#include "api/memory_ref.h"

struct cxx_unordered_set_OffsetTable {
    uint32_t __bucket_list_;
    uint32_t __p1_;
    uint32_t __p2_;
    uint32_t __p3_;

    // bucket_list
    uint32_t __bucket_list__value_;
    uint32_t __bucket_list__index_;

    // pair
    uint32_t __pair__next_;
    uint32_t __pair__hash_;
    uint32_t __pair__data_;
};

struct cxx_unordered_set_SizeTable {
    uint32_t THIS;
    uint32_t bucket_list_size;
    uint32_t pair_size;
};

extern struct cxx_unordered_set_OffsetTable __cxx_unordered_set_offset__;
extern struct cxx_unordered_set_SizeTable __cxx_unordered_set_size__;

namespace cxx {

class unordered_set : public api::MemoryRef {
public:
    unordered_set(uint64_t v) : api::MemoryRef(v) {}
    unordered_set(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    unordered_set(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    unordered_set(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(unordered_set& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(unordered_set& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __bucket_list() { return Ptr() + OFFSET(cxx_unordered_set, __bucket_list_); }
    inline uint64_t __p1() { return VALUEOF(cxx_unordered_set, __p1_); }
    inline uint64_t __p2() { return VALUEOF(cxx_unordered_set, __p2_); }
    inline uint64_t __p3() { return VALUEOF(cxx_unordered_set, __p3_); }

    class pair : public api::MemoryRef {
    public:
        pair(uint64_t v) : api::MemoryRef(v) {}
        pair(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        pair(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        pair(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        inline bool operator==(pair& ref) const { return Ptr() == ref.Ptr(); }
        inline bool operator!=(pair& ref) const { return Ptr() != ref.Ptr(); }

        inline uint64_t __next() { return VALUEOF(cxx_unordered_set, __pair__next_); }
        inline uint64_t __hash() { return VALUEOF(cxx_unordered_set, __pair__hash_); }
        inline uint64_t __data() { return Ptr() + OFFSET(cxx_unordered_set, __pair__data_); }
    };

    class iterator {
    public:
        iterator(uint64_t pair)
            : pair_cache(pair) { pair_cache.Prepare(false); }
        iterator& operator++();
        bool operator==(iterator other) const;
        bool operator!=(iterator other) const;
        uint64_t operator*();
    private:
        unordered_set::pair pair_cache = 0x0;
    };

    iterator begin();
    iterator end();
    uint64_t size();
};

} // namespace cxx

#endif  // LLVM_CXX_UNORDERED_SET_H_
