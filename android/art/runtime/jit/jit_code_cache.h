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

#ifndef ANDROID_ART_RUNTIME_JIT_JIT_CODE_CACHE_H_
#define ANDROID_ART_RUNTIME_JIT_JIT_CODE_CACHE_H_

#include "api/memory_ref.h"
#include "runtime/art_method.h"
#include "runtime/jit/jit_memory_region.h"
#include "base/mem_map.h"
#include "cxx/map.h"

struct JitCodeCache_OffsetTable {
    uint32_t code_map_;
    uint32_t exec_pages_;
    uint32_t shared_region_;
    uint32_t private_region_;
    uint32_t jni_stubs_map_;
    uint32_t method_code_map_;
    uint32_t zygote_exec_pages_;
    uint32_t zygote_map_;
};

extern struct JitCodeCache_OffsetTable __JitCodeCache_offset__;

struct JniStubsMapPair_OffsetTable {
    uint32_t first;
    uint32_t second;
};

extern struct JniStubsMapPair_OffsetTable __JniStubsMapPair_offset__;

struct JniStubData_OffsetTable {
    uint32_t code_;
    uint32_t methods_;
};

extern struct JniStubData_OffsetTable __JniStubData_offset__;

struct ZygoteMap_OffsetTable {
    uint32_t array_;
    uint32_t size_;
    uint32_t region_;
    uint32_t compilation_state_;
};

extern struct ZygoteMap_OffsetTable __ZygoteMap_offset__;

namespace art {
namespace jit {

class ZygoteMap : public api::MemoryRef {
public:
    ZygoteMap(uint64_t v) : api::MemoryRef(v) {}
    ZygoteMap(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ZygoteMap(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ZygoteMap(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init30();
    inline uint64_t array() { return VALUEOF(ZygoteMap, array_); }
    inline uint64_t size() { return VALUEOF(ZygoteMap, size_); }

    uint64_t GetCodeFor(ArtMethod* method, uint64_t pc);
};

class JitCodeCache : public api::MemoryRef {
public:
    JitCodeCache(uint64_t v) : api::MemoryRef(v) {}
    JitCodeCache(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    JitCodeCache(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    JitCodeCache(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init26();
    static void Init28();
    static void Init29();
    static void Init30();
    static void Init31();
    inline uint64_t code_map() { return VALUEOF(JitCodeCache, code_map_); }
    inline uint64_t exec_pages() { return Ptr() + OFFSET(JitCodeCache, exec_pages_); }
    inline uint64_t shared_region() { return Ptr() + OFFSET(JitCodeCache, shared_region_); }
    inline uint64_t private_region() { return Ptr() + OFFSET(JitCodeCache, private_region_); }
    inline uint64_t jni_stubs_map() { return Ptr() + OFFSET(JitCodeCache, jni_stubs_map_); }
    inline uint64_t method_code_map() { return Ptr() + OFFSET(JitCodeCache, method_code_map_); }
    inline uint64_t zygote_exec_pages() { return Ptr() + OFFSET(JitCodeCache, zygote_exec_pages_); }
    inline uint64_t zygote_map() { return Ptr() + OFFSET(JitCodeCache, zygote_map_); }

    MemMap& GetCodeMap();
    MemMap& GetExecPages();
    JitMemoryRegion& GetSharedRegion();
    JitMemoryRegion& GetPrivateRegion();
    cxx::map& GetJniStubsMap();
    cxx::map& GetMethodCodeMap();
    MemMap& GetZygoteExecPages();
    ZygoteMap& GetZygoteMap();

    OatQuickMethodHeader LookupMethodHeader(uint64_t pc, ArtMethod* method);
    OatQuickMethodHeader LookupMethodCodeMap(uint64_t pc, ArtMethod* method);
    bool PrivateRegionContainsPc(uint64_t pc);
    bool ContainsPc(uint64_t pc);

    class JniStubKey : public api::MemoryRef {
    public:
        JniStubKey(uint64_t v) : api::MemoryRef(v) {}
        JniStubKey(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        JniStubKey(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        JniStubKey(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    };

    class JniStubData : public api::MemoryRef {
    public:
        JniStubData(uint64_t v) : api::MemoryRef(v) {}
        JniStubData(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        JniStubData(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        JniStubData(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}

        static void Init28();
        inline uint64_t code() { return VALUEOF(JniStubData, code_); }
        inline uint64_t methods() { return Ptr() + OFFSET(JniStubData, methods_); }
    };

    class JniStubsMapPair : public api::MemoryRef {
    public:
        JniStubsMapPair(uint64_t v) : api::MemoryRef(v) {}
        JniStubsMapPair(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        JniStubsMapPair(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        JniStubsMapPair(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}

        static void Init28();
        inline uint64_t first() { return VALUEOF(JniStubsMapPair, first); }
        inline uint64_t second() { return Ptr() + OFFSET(JniStubsMapPair, second); }
    };
private:
    // quick memoryref cache
    MemMap code_map_cache = 0x0;
    MemMap exec_pages_cache = 0x0;
    JitMemoryRegion shared_region_cache = 0x0;
    JitMemoryRegion private_region_cache = 0x0;
    cxx::map jni_stubs_map_cache = 0x0;
    cxx::map method_code_map_cache = 0x0;
    MemMap zygote_exec_pages_cache = 0x0;
    ZygoteMap zygote_map_cache = 0x0;
};

} // namespace jit
} // namespace art

#endif // ANDROID_ART_RUNTIME_JIT_JIT_CODE_CACHE_H_
