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

#ifndef LLVM_CXX_MAP_H_
#define LLVM_CXX_MAP_H_

#include "api/memory_ref.h"
#include <vector>

struct cxx_map_OffsetTable {
    uint32_t __begin_node_;
    uint32_t __pair1_;
    uint32_t __pair3_;
};

struct cxx_map_SizeTable {
    uint32_t THIS;
};

extern struct cxx_map_OffsetTable __cxx_map_offset__;
extern struct cxx_map_SizeTable __cxx_map_size__;

struct cxx_map_pair_OffsetTable {
    uint32_t __left_;
    uint32_t __right_;
    uint32_t __parent_;
    uint32_t __color_;
    uint32_t __data_;
};

struct cxx_map_pair_SizeTable {
    uint32_t THIS;
};

extern struct cxx_map_pair_OffsetTable __cxx_map_pair_offset__;
extern struct cxx_map_pair_SizeTable __cxx_map_pair_size__;

namespace cxx {

class map : public api::MemoryRef {
public:
    map() : api::MemoryRef() {}
    map(uint64_t v) : api::MemoryRef(v) {}
    map(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    map(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    map(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(map& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(map& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __begin_node() { return VALUEOF(cxx_map, __begin_node_); }
    inline uint64_t __pair1() { return VALUEOF(cxx_map, __pair1_); }
    inline uint64_t __pair3() { return VALUEOF(cxx_map, __pair3_); }

    class pair : public api::MemoryRef {
    public:
        pair() : api::MemoryRef() {}
        pair(uint64_t v) : api::MemoryRef(v) {}
        pair(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        pair(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        pair(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        inline bool operator==(pair& ref) const { return Ptr() == ref.Ptr(); }
        inline bool operator!=(pair& ref) const { return Ptr() != ref.Ptr(); }

        static void Init();
        inline uint64_t __left() { return VALUEOF(cxx_map_pair, __left_); }
        inline uint64_t __right() { return VALUEOF(cxx_map_pair, __right_); }
        inline uint64_t __parent() { return VALUEOF(cxx_map_pair, __parent_); }
        inline uint64_t __color() { return VALUEOF(cxx_map_pair, __color_); }
        inline uint64_t __data() { return Ptr() + OFFSET(cxx_map_pair, __data_); }
    };

    class iterator {
    public:
        iterator(map* m, uint64_t pos)
            : __map_(m), current(pos) {}
        iterator& operator++();
        bool operator==(iterator other) const;
        bool operator!=(iterator other) const;
        uint64_t operator*();
        void traversal(pair& p);
    private:
        map* __map_;
        uint64_t current;
    };

    iterator begin();
    iterator end();
    uint64_t size();

private:
    std::vector<pair> cache;
};

} // namespace cxx

#endif  // LLVM_CXX_MAP_H_
