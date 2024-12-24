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

#ifndef ANDROID_ART_RUNTIME_MIRROR_DEXCACHE_H_
#define ANDROID_ART_RUNTIME_MIRROR_DEXCACHE_H_

#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/string.h"
#include "dex/dex_file.h"
#include <string>

struct DexCache_OffsetTable {
    uint32_t class_loader_;
    uint32_t location_;
    uint32_t dex_file_;
    uint32_t preresolved_strings_;
    uint32_t resolved_call_sites_;
    uint32_t resolved_fields_;
    uint32_t resolved_method_types_;
    uint32_t resolved_methods_;
    uint32_t resolved_types_;
    uint32_t strings_;
    uint32_t num_preresolved_strings_;
    uint32_t num_resolved_call_sites_;
    uint32_t num_resolved_fields_;
    uint32_t num_resolved_method_types_;
    uint32_t num_resolved_methods_;
    uint32_t num_resolved_types_;
    uint32_t num_strings_;
};

struct DexCache_SizeTable {
    uint32_t THIS;
};

extern struct DexCache_OffsetTable __DexCache_offset__;
extern struct DexCache_SizeTable __DexCache_size__;

namespace art {
namespace mirror {

class DexCache : public Object {
public:
    DexCache(uint32_t v) : Object(v) {}
    DexCache(const Object& ref) : Object(ref) {}
    DexCache(uint32_t v, Object* ref) : Object(v, ref) {}

    inline bool operator==(Object& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Object& ref) { return Ptr() != ref.Ptr(); }
    inline bool operator==(uint32_t v) { return Ptr() == v; }
    inline bool operator!=(uint32_t v) { return Ptr() != v; }

    static void Init();
    static void Init23();
    static void Init24();
    static void Init26();
    static void Init30();
    inline uint32_t class_loader() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(DexCache, class_loader_)); }
    inline uint32_t location() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(DexCache, location_)); }
    inline uint64_t dex_file() { return *reinterpret_cast<uint64_t *>(Real() + OFFSET(DexCache, dex_file_)); }

    inline DexFile& GetDexFile() { return QUICK_CACHE(dex_file); }
    inline String& GetLocation() { return QUICK_CACHE(location); }

private:
    // quick memoryref cache
    DEFINE_QUICK_CACHE(DexFile, dex_file);
    DEFINE_QUICK_CACHE(String, location);
};

} // namespace mirror
} // namespace art

#endif  // ANDROID_ART_RUNTIME_MIRROR_DEXCACHE_H_
