/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_ART_RUNTIME_CLASS_LINKER_H_
#define ANDROID_ART_RUNTIME_CLASS_LINKER_H_

#include "api/memory_ref.h"
#include "cxx/list.h"
#include "cxx/unordered_map.h"

struct ClassLinker_OffsetTable {
    uint32_t dex_caches_;
};

struct ClassLinker_SizeTable {
    uint32_t THIS;
};

extern struct ClassLinker_OffsetTable __ClassLinker_offset__;
extern struct ClassLinker_SizeTable __ClassLinker_size__;

struct DexCacheData_OffsetTable {
    uint32_t weak_root;
    uint32_t dex_file;
    uint32_t class_table;
    uint32_t registration_index;
};

struct DexCacheData_SizeTable {
    uint32_t THIS;
};

extern struct DexCacheData_OffsetTable __DexCacheData_offset__;
extern struct DexCacheData_SizeTable __DexCacheData_size__;

namespace art {

class ClassLinker : public api::MemoryRef {
public:
    ClassLinker(uint64_t v) : api::MemoryRef(v) {}
    ClassLinker(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ClassLinker(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ClassLinker(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> ClassLinker(U *v) : api::MemoryRef(v) {}
    template<typename U> ClassLinker(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t dex_caches() { return Ptr() + OFFSET(ClassLinker, dex_caches_); }

    class DexCacheData : public api::MemoryRef {
        DexCacheData(uint64_t v) : api::MemoryRef(v) {}
        DexCacheData(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        DexCacheData(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        DexCacheData(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        template<typename U> DexCacheData(U *v) : api::MemoryRef(v) {}
        template<typename U> DexCacheData(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init();
        static void Init33();
    };

    uint32_t GetDexCacheCount();
    cxx::list& GetDexCachesData();
    cxx::unordered_map& GetDexCachesData_v33();
private:
    // quick memoryref cache
    cxx::list dex_caches_cache = 0x0;
    cxx::unordered_map dex_caches_v33_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_CLASS_LINKER_H_