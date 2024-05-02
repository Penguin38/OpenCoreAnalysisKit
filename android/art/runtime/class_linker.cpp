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

#include "runtime/class_linker.h"
#include "android.h"

struct ClassLinker_OffsetTable __ClassLinker_offset__;
struct ClassLinker_SizeTable __ClassLinker_size__;
struct DexCacheData_OffsetTable __DexCacheData_offset__;
struct DexCacheData_SizeTable __DexCacheData_size__;

namespace art {

void ClassLinker::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __ClassLinker_offset__ = {
            .dex_caches_ = 56,
        };
    } else {
        __ClassLinker_offset__ = {
            .dex_caches_ = 28,
        };
    }
}

void ClassLinker::DexCacheData::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .dex_file = 8,
            .class_table = 16,
        };
    } else {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .dex_file = 4,
            .class_table = 8,
        };
    }
}

void ClassLinker::DexCacheData::Init33() {
    if (CoreApi::GetPointSize() == 64) {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .class_table = 8,
            .registration_index = 16,
        };
    } else {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .class_table = 4,
            .registration_index = 8,
        };
    }
}

uint32_t ClassLinker::GetDexCacheCount() {
    if (Android::Sdk() < Android::TIRAMISU) {
        return GetDexCachesData().size();
    } else {
        return GetDexCachesData_v33().size();
    }
}

cxx::list& ClassLinker::GetDexCachesData() {
    if (!dex_caches_cache.Ptr()) {
        dex_caches_cache = dex_caches();
        dex_caches_cache.copyRef(this);
        dex_caches_cache.Prepare(false);
    }
    return dex_caches_cache;
}

cxx::unordered_map& ClassLinker::GetDexCachesData_v33() {
    if (!dex_caches_v33_cache.Ptr()) {
        dex_caches_v33_cache = dex_caches();
        dex_caches_v33_cache.copyRef(this);
        dex_caches_v33_cache.Prepare(false);
    }
    return dex_caches_v33_cache;
}

} //namespace art