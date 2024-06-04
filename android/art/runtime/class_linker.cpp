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
#include "runtime/runtime.h"
#include "android.h"

struct ClassLinker_OffsetTable __ClassLinker_offset__;
struct ClassLinker_SizeTable __ClassLinker_size__;
struct DexCacheData_OffsetTable __DexCacheData_offset__;
struct DexCacheData_SizeTable __DexCacheData_size__;

namespace art {

void ClassLinker::Init26() {
    if (CoreApi::GetPointSize() == 64) {
        __ClassLinker_offset__ = {
            .dex_caches_ = 48,
        };
    } else {
        __ClassLinker_offset__ = {
            .dex_caches_ = 24,
        };
    }
}

void ClassLinker::Init28() {
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

void ClassLinker::DexCacheData::Init26() {
    if (CoreApi::GetPointSize() == 64) {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .dex_file = 8,
            .class_table = 24,
        };
    } else {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .dex_file = 4,
            .class_table = 12,
        };
    }
}

void ClassLinker::DexCacheData::Init28() {
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

std::vector<std::unique_ptr<ClassLinker::DexCacheData>>& ClassLinker::GetDexCacheDatas() {
    if (!dex_caches_second_cache.empty())
        return dex_caches_second_cache;

    Runtime& runtime = art::Runtime::Current();
    JavaVMExt& vm = runtime.GetJavaVM();

    if (Android::Sdk() < Android::TIRAMISU) {
        for (const auto& value : GetDexCachesData()) {
            // std::list<DexCacheData> dex_caches_ GUARDED_BY(Locks::dex_lock_);
            std::unique_ptr<ClassLinker::DexCacheData> data = std::make_unique<ClassLinker::DexCacheData>(value);
            data->InitCache(vm.Decode(data->weak_root()), data->dex_file());
            dex_caches_second_cache.push_back(std::move(data));
        }
    } else {
        for (const auto& value : GetDexCachesData_v33()) {
            // std::unordered_map<const DexFile*, DexCacheData> dex_caches_ GUARDED_BY(Locks::dex_lock_);
            api::MemoryRef ref = value;
            std::unique_ptr<ClassLinker::DexCacheData> data = std::make_unique<ClassLinker::DexCacheData>(CoreApi::GetPointSize() / 8 + value);
            data->InitCache(vm.Decode(data->weak_root()), ref.valueOf());
            dex_caches_second_cache.push_back(std::move(data));
        }
    }
    return dex_caches_second_cache;
}

bool ClassLinker::IsQuickGenericJniStub(uint64_t entry_point) {
    return false;
}

bool ClassLinker::IsQuickResolutionStub(uint64_t entry_point) {
    return false;
}

bool ClassLinker::IsQuickToInterpreterBridge(uint64_t entry_point) {
    return false;
}

} //namespace art
