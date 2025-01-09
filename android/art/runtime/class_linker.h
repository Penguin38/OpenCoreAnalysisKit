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

#ifndef ANDROID_ART_RUNTIME_CLASS_LINKER_H_
#define ANDROID_ART_RUNTIME_CLASS_LINKER_H_

#include "api/memory_ref.h"
#include "cxx/list.h"
#include "cxx/vector.h"
#include "cxx/unordered_map.h"
#include "runtime/mirror/dex_cache.h"
#include <vector>
#include <memory>

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

    static void Init();
    static void Init23();
    static void Init24();
    static void Init26();
    static void Init28();
    inline uint64_t dex_caches() { return Ptr() + OFFSET(ClassLinker, dex_caches_); }

    class DexCacheData : public api::MemoryRef {
	public:
        DexCacheData(uint64_t v) : api::MemoryRef(v) {}
        DexCacheData(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        DexCacheData(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        DexCacheData(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init24();
        static void Init26();
        static void Init28();
        static void Init33();
        inline uint64_t weak_root() { return VALUEOF(DexCacheData, weak_root); }
        inline uint64_t dex_file() { return VALUEOF(DexCacheData, dex_file); }

        void InitCache(mirror::Object dex_cache, uint64_t dex_file) {
            dex_cache_cache = dex_cache;
            dex_file_cache = dex_file;
        }
        mirror::DexCache& GetDexCache() { return dex_cache_cache; }
        DexFile& GetDexFile() { return dex_file_cache; }
    private:
        // quick memoryref cache
        mirror::DexCache dex_cache_cache = 0x0;
        DexFile dex_file_cache = 0x0;
    };

    uint32_t GetDexCacheCount();
    cxx::vector& GetDexCachesData_v23();
    cxx::list& GetDexCachesData();
    cxx::unordered_map& GetDexCachesData_v33();
    std::vector<std::unique_ptr<DexCacheData>>& GetDexCacheDatas();
    bool IsQuickGenericJniStub(uint64_t entry_point);
    bool IsQuickResolutionStub(uint64_t entry_point);
    bool IsQuickToInterpreterBridge(uint64_t entry_point);
    void CleanCache() {
        dex_caches_second_cache.clear();
    }
private:
    // quick memoryref cache
    cxx::vector dex_caches_v23_cache = 0x0;
    cxx::list dex_caches_cache = 0x0;
    cxx::unordered_map dex_caches_v33_cache = 0x0;

    // second cache
    std::vector<std::unique_ptr<DexCacheData>> dex_caches_second_cache;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_CLASS_LINKER_H_
