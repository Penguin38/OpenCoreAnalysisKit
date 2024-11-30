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

#include "api/core.h"
#include "common/elf.h"
#include "android.h"
#include "runtime/jit/jit_code_cache.h"
#include "cxx/vector.h"
#include <limits>

struct JitCodeCache_OffsetTable __JitCodeCache_offset__;
struct JniStubsMapPair_OffsetTable __JniStubsMapPair_offset__;
struct JniStubData_OffsetTable __JniStubData_offset__;
struct ZygoteMap_OffsetTable __ZygoteMap_offset__;

namespace art {
namespace jit {

void JitCodeCache::Init() {
    Android::RegisterSdkListener(Android::O, art::jit::JitCodeCache::Init26);
    Android::RegisterSdkListener(Android::P, art::jit::JitCodeCache::Init28);
    Android::RegisterSdkListener(Android::Q, art::jit::JitCodeCache::Init29);
    Android::RegisterSdkListener(Android::R, art::jit::JitCodeCache::Init30);
    Android::RegisterSdkListener(Android::S, art::jit::JitCodeCache::Init31);
    Android::RegisterSdkListener(Android::V, art::jit::JitCodeCache::Init35);

    Android::RegisterSdkListener(Android::P, art::jit::JitCodeCache::JniStubData::Init28);
    Android::RegisterSdkListener(Android::P, art::jit::JitCodeCache::JniStubsMapPair::Init28);

    Android::RegisterSdkListener(Android::R, art::jit::ZygoteMap::Init30);
}

void ZygoteMap::Init30() {
    if (CoreApi::Bits() == 64) {
        __ZygoteMap_offset__ = {
            .array_ = 0,
            .size_ = 8,
            .region_ = 16,
            .compilation_state_ = 24,
        };
    } else {
        __ZygoteMap_offset__ = {
            .array_ = 0,
            .size_ = 4,
            .region_ = 8,
            .compilation_state_ = 12,
        };
    }
}

void JitCodeCache::Init26() {
    if (CoreApi::Bits() == 64) {
        __JitCodeCache_offset__ = {
            .code_map_ = 96,
            .method_code_map_ = 136,
        };
    } else {
        __JitCodeCache_offset__ = {
            .code_map_ = 60,
            .method_code_map_ = 80,
        };
    }
}

void JitCodeCache::Init28() {
    if (CoreApi::Bits() == 64) {
        __JitCodeCache_offset__ = {
            .code_map_ = 96,
            .jni_stubs_map_ = 136,
            .method_code_map_ = 160,
        };
    } else {
        __JitCodeCache_offset__ = {
            .code_map_ = 60,
            .jni_stubs_map_ = 80,
            .method_code_map_ = 92,
        };
    }
}

void JitCodeCache::Init29() {
    if (CoreApi::Bits() == 64) {
        __JitCodeCache_offset__ = {
            .exec_pages_ = 90,
            .jni_stubs_map_ = 312,
            .method_code_map_ = 336,
            .zygote_exec_pages_ = 1008,
        };
    } else {
        __JitCodeCache_offset__ = {
            .exec_pages_ = 88,
            .jni_stubs_map_ = 180,
            .method_code_map_ = 192,
            .zygote_exec_pages_ = 624,
        };
    }
}

void JitCodeCache::Init30() {
    if (CoreApi::Bits() == 64) {
        __JitCodeCache_offset__ = {
            .shared_region_ = 32,
            .private_region_ = 392,
            .jni_stubs_map_ = 752,
            .method_code_map_ = 776,
            .zygote_map_ = 872,
        };
    } else {
        __JitCodeCache_offset__ = {
            .shared_region_ = 20,
            .private_region_ = 216,
            .jni_stubs_map_ = 412,
            .method_code_map_ = 424,
            .zygote_map_ = 472,
        };
    }
}

void JitCodeCache::Init31() {
    if (CoreApi::Bits() == 64) {
        __JitCodeCache_offset__ = {
            .shared_region_ = 32,
            .private_region_ = 392,
            .jni_stubs_map_ = 752,
            .method_code_map_ = 776,
            .zygote_map_ = 944,
        };
    } else {
        __JitCodeCache_offset__ = {
            .shared_region_ = 20,
            .private_region_ = 216,
            .jni_stubs_map_ = 412,
            .method_code_map_ = 424,
            .zygote_map_ = 508,
        };
    }
}

void JitCodeCache::Init35() {
    if (CoreApi::Bits() == 64) {
        __JitCodeCache_offset__ = {
            .shared_region_ = 40,
            .private_region_ = 400,
            .jni_stubs_map_ = 760,
            .method_code_map_ = 784,
            .zygote_map_ = 880,
        };
    }
}

void JitCodeCache::JniStubData::Init28() {
    if (CoreApi::Bits() == 64) {
        __JniStubData_offset__ = {
            .code_ = 0,
            .methods_ = 8,
        };
    } else {
        __JniStubData_offset__ = {
            .code_ = 0,
            .methods_ = 4,
        };
    }
}

void JitCodeCache::JniStubsMapPair::Init28() {
    if (CoreApi::Bits() == 64) {
        __JniStubsMapPair_offset__ = {
            .first = 0,
            .second = 16,
        };
    } else {
        __JniStubsMapPair_offset__ = {
            .first = 0,
            .second = 8,
        };
    }
}

MemMap& JitCodeCache::GetCodeMap() {
    if (!code_map_cache.Ptr()) {
        code_map_cache = code_map();
        code_map_cache.copyRef(this);
        code_map_cache.Prepare(false);
    }
    return code_map_cache;
}

MemMap& JitCodeCache::GetExecPages() {
    if (!exec_pages_cache.Ptr()) {
        exec_pages_cache = exec_pages();
        exec_pages_cache.copyRef(this);
    }
    return exec_pages_cache;
}

JitMemoryRegion& JitCodeCache::GetSharedRegion() {
    if (!shared_region_cache.Ptr()) {
        shared_region_cache = shared_region();
        shared_region_cache.copyRef(this);
    }
    return shared_region_cache;
}

JitMemoryRegion& JitCodeCache::GetPrivateRegion() {
    if (!private_region_cache.Ptr()) {
        private_region_cache = private_region();
        private_region_cache.copyRef(this);
    }
    return private_region_cache;
}

cxx::map& JitCodeCache::GetJniStubsMap() {
    if (!jni_stubs_map_cache.Ptr()) {
        jni_stubs_map_cache = jni_stubs_map();
        jni_stubs_map_cache.copyRef(this);
    }
    return jni_stubs_map_cache;
}

cxx::map& JitCodeCache::GetMethodCodeMap() {
    if (!method_code_map_cache.Ptr()) {
        method_code_map_cache = method_code_map();
        method_code_map_cache.copyRef(this);
    }
    return method_code_map_cache;
}

MemMap& JitCodeCache::GetZygoteExecPages() {
    if (!zygote_exec_pages_cache.Ptr()) {
        zygote_exec_pages_cache = zygote_exec_pages();
        zygote_exec_pages_cache.copyRef(this);
    }
    return zygote_exec_pages_cache;
}

ZygoteMap& JitCodeCache::GetZygoteMap() {
    if (!zygote_map_cache.Ptr()) {
        zygote_map_cache = zygote_map();
        zygote_map_cache.copyRef(this);
    }
    return zygote_map_cache;
}

bool JitCodeCache::PrivateRegionContainsPc(uint64_t pc) {
    return GetPrivateRegion().IsInExecSpace(pc);
}

bool JitCodeCache::ContainsPc(uint64_t pc) {
    if (Android::Sdk() >= Android::R) {
        return PrivateRegionContainsPc(pc) || GetSharedRegion().IsInExecSpace(pc);
    } else if (Android::Sdk() >= Android::Q) {
        return GetExecPages().HasAddress(pc) || GetZygoteExecPages().HasAddress(pc);
    } else {
        return GetCodeMap().HasAddress(pc);
    }
}

uint64_t JitCodeCache::GetJniStubCode(ArtMethod& method) {
    for (const auto& value : GetJniStubsMap()) {
        JniStubsMapPair pair = value;
        JniStubData data = pair.second();
        cxx::vector methods_(data.methods(), data);
        methods_.SetEntrySize(CoreApi::GetPointSize());
        for (const auto& m : methods_) {
            api::MemoryRef ref = m;
            if (ref.valueOf() == method.Ptr()) {
                return data.code();
            }
        }
    }
    return 0x0;
}

uint64_t ZygoteMap::GetCodeFor(ArtMethod& method, uint64_t pc) {
    if (!size()) {
        return 0x0;
    }

    std::hash<uint64_t> hf;
    uint64_t index = hf(method.Ptr()) & (size() - 1u);

    uint32_t point_size = CoreApi::GetPointSize();
    api::MemoryRef array_ = array();
    while (true) {
        ArtMethod m = array_.valueOf(index * 2 * point_size);
        uint64_t code_ptr = array_.valueOf(index * 2 * point_size + point_size);
        if (!m.Ptr()) {
            return 0x0;
        }
        if (m == method) {
            if (!code_ptr) {
                return 0x0;
            }
            OatQuickMethodHeader code_header = OatQuickMethodHeader::FromCodePointer(code_ptr);
            if (pc && !code_header.Contains(pc)) {
                return 0x0;
            }
            return code_ptr;
        }
        index = (index + 1) & (size() - 1);
    }
    return 0x0;
}

OatQuickMethodHeader JitCodeCache::LookupMethodCodeMap(uint64_t pc, ArtMethod& /*method*/) {
    if (!GetMethodCodeMap().size()) {
        return 0x0;
    }

    uint64_t code_ptr;
    uint32_t point_size = CoreApi::GetPointSize();
    uint64_t nice = std::numeric_limits<uint64_t>::max();
    for (const auto& value : GetMethodCodeMap()) {
        api::MemoryRef ref = value;
        uint64_t tmp_pc = ref.valueOf();
        if (pc < tmp_pc)
            continue;

        if (pc - tmp_pc <= nice) {
            nice = pc - tmp_pc;
            code_ptr = tmp_pc;
        }
    }

    return OatQuickMethodHeader::FromCodePointer(code_ptr);
}

OatQuickMethodHeader JitCodeCache::LookupMethodHeader(uint64_t pc, ArtMethod& method) {
    OatQuickMethodHeader method_header = 0x0;
    if (CoreApi::GetMachine() == EM_ARM) {
        pc &= (CoreApi::GetPointMask() - 1);
    }

    if (!ContainsPc(pc)) {
        return method_header;
    }

    // SafeMap<JniStubKey, JniStubData> jni_stubs_map_ GUARDED_BY(lock_);
    // SafeMap<const void*, ArtMethod*> method_code_map_ GUARDED_BY(lock_);
    if (Android::Sdk() >= Android::P) {
        if (method.Ptr() && method.IsNative()) {
            uint64_t entry_point = GetJniStubCode(method);
            if (!entry_point)
                return 0x0;

            method_header = OatQuickMethodHeader::FromCodePointer(entry_point);
            if (!method_header.Contains(pc)) {
                return 0x0;
            }
        } else {
            if (Android::Sdk() >= Android::R) {
                if (GetSharedRegion().IsInExecSpace(pc)) {
                    uint64_t code_ptr = GetZygoteMap().GetCodeFor(method, pc);
                    if (code_ptr) {
                        return OatQuickMethodHeader::FromCodePointer(code_ptr);
                    }
                }
            }

            method_header = LookupMethodCodeMap(pc, method);
            if (!method_header.Contains(pc)) {
                return 0x0;
            }
        }
    } else {
        method_header = LookupMethodCodeMap(pc, method);
        if (!method_header.Contains(pc)) {
            return 0x0;
        }
    }

    return method_header;
}

} // namespace jit
} // namespace art

