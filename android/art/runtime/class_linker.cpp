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

#include "runtime/class_linker.h"
#include "runtime/runtime.h"
#include "runtime/entrypoints/runtime_asm_entrypoints.h"
#include "android.h"

struct ClassLinker_OffsetTable __ClassLinker_offset__;
struct ClassLinker_SizeTable __ClassLinker_size__;
struct DexCacheData_OffsetTable __DexCacheData_offset__;
struct DexCacheData_SizeTable __DexCacheData_size__;

namespace art {

void ClassLinker::Init() {
    Android::RegisterSdkListener(Android::M, art::ClassLinker::Init23);
    Android::RegisterSdkListener(Android::N, art::ClassLinker::Init24);
    Android::RegisterSdkListener(Android::O, art::ClassLinker::Init26);
    Android::RegisterSdkListener(Android::P, art::ClassLinker::Init28);

    Android::RegisterSdkListener(Android::N, art::ClassLinker::DexCacheData::Init24);
    Android::RegisterSdkListener(Android::O, art::ClassLinker::DexCacheData::Init26);
    Android::RegisterSdkListener(Android::P, art::ClassLinker::DexCacheData::Init28);
    Android::RegisterSdkListener(Android::T, art::ClassLinker::DexCacheData::Init33);
}

void ClassLinker::Init23() {
    if (CoreApi::Bits() == 64) {
        __ClassLinker_offset__ = {
            .dex_caches_ = 120,
        };
    } else {
        __ClassLinker_offset__ = {
            .dex_caches_ = 76,
        };
    }
}

void ClassLinker::Init24() {
    if (CoreApi::Bits() == 64) {
        __ClassLinker_offset__ = {
            .dex_caches_ = 96,
        };
    } else {
        __ClassLinker_offset__ = {
            .dex_caches_ = 64,
        };
    }
}

void ClassLinker::Init26() {
    if (CoreApi::Bits() == 64) {
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
    if (CoreApi::Bits() == 64) {
        __ClassLinker_offset__ = {
            .dex_caches_ = 56,
        };
    } else {
        __ClassLinker_offset__ = {
            .dex_caches_ = 28,
        };
    }
}

void ClassLinker::DexCacheData::Init24() {
    if (CoreApi::Bits() == 64) {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .dex_file = 8,
        };
    } else {
        __DexCacheData_offset__ = {
            .weak_root = 0,
            .dex_file = 4,
        };
    }
}

void ClassLinker::DexCacheData::Init26() {
    if (CoreApi::Bits() == 64) {
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
    if (CoreApi::Bits() == 64) {
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
    if (CoreApi::Bits() == 64) {
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
    if (Android::Sdk() >= Android::T) {
        return GetDexCachesData_v33().size();
    } else if (Android::Sdk() >= Android::N) {
        return GetDexCachesData().size();
    } else {
        return GetDexCachesData_v23().size();
    }
}

cxx::vector& ClassLinker::GetDexCachesData_v23() {
    if (!dex_caches_v23_cache.Ptr()) {
        dex_caches_v23_cache = dex_caches();
        dex_caches_v23_cache.copyRef(this);
        dex_caches_v23_cache.Prepare(false);
        dex_caches_v23_cache.SetEntrySize(4);
    }
    return dex_caches_v23_cache;
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

    if (Android::Sdk() >= Android::T) {
        for (const auto& value : GetDexCachesData_v33()) {
            // std::unordered_map<const DexFile*, DexCacheData> dex_caches_ GUARDED_BY(Locks::dex_lock_);
            api::MemoryRef ref = value;
            std::unique_ptr<ClassLinker::DexCacheData> data = std::make_unique<ClassLinker::DexCacheData>(value + 0x8);
            data->InitCache(vm.Decode(data->weak_root()), ref.valueOf());
            dex_caches_second_cache.push_back(std::move(data));
        }
    } else if (Android::Sdk() >= Android::N) {
        for (const auto& value : GetDexCachesData()) {
            // std::list<DexCacheData> dex_caches_ GUARDED_BY(Locks::dex_lock_);
            std::unique_ptr<ClassLinker::DexCacheData> data = std::make_unique<ClassLinker::DexCacheData>(value);
            data->InitCache(vm.Decode(data->weak_root()), data->dex_file());
            dex_caches_second_cache.push_back(std::move(data));
        }
    } else {
        for (const auto& value : GetDexCachesData_v23()) {
            // std::vector<GcRoot<mirror::DexCache>> dex_caches_ GUARDED_BY(dex_lock_);
            api::MemoryRef ref = value;
            std::unique_ptr<ClassLinker::DexCacheData> data = std::make_unique<ClassLinker::DexCacheData>(0x0);
            mirror::DexCache dex_cache = ref.value32Of();
            data->InitCache(dex_cache.Ptr(), dex_cache.GetDexFile().Ptr());
            dex_caches_second_cache.push_back(std::move(data));
        }
    }
    return dex_caches_second_cache;
}

bool ClassLinker::IsQuickGenericJniStub(uint64_t entry_point) {
    return entry_point && (entry_point == GetQuickGenericJniStub());
}

bool ClassLinker::IsQuickResolutionStub(uint64_t entry_point) {
    return entry_point && (entry_point == GetQuickResolutionStub());
}

bool ClassLinker::IsQuickToInterpreterBridge(uint64_t entry_point) {
    return entry_point && (entry_point == GetQuickToInterpreterBridge());
}

} //namespace art
