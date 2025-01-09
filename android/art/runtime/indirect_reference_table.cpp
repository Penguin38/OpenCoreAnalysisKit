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

#include "logger/log.h"
#include "runtime/indirect_reference_table.h"
#include "android.h"

struct IrtEntry_OffsetTable __IrtEntry_offset__;
struct IrtEntry_SizeTable __IrtEntry_size__;
struct IndirectReferenceTable_OffsetTable __IndirectReferenceTable_offset__;

namespace art {

void IndirectReferenceTable::Init() {
    Android::RegisterSdkListener(Android::M, art::IndirectReferenceTable::Init23);
    Android::RegisterSdkListener(Android::Q, art::IndirectReferenceTable::Init29);
    Android::RegisterSdkListener(Android::U, art::IndirectReferenceTable::Init34);

    Android::RegisterSdkListener(Android::M, art::IrtEntry::Init23);
    Android::RegisterSdkListener(Android::T, art::IrtEntry::Init33);
}

void IrtEntry::Init23() {
    __IrtEntry_offset__ = {
        .serial_ = 0,
        .references_ = 4,
    };
    __IrtEntry_size__ = {
        .THIS = 16,
    };
}

void IrtEntry::Init33() {
    __IrtEntry_offset__ = {
        .serial_ = 0,
        .references_ = 4,
    };
    __IrtEntry_size__ = {
        .THIS = 8,
    };
}

void IndirectReferenceTable::Init23() {
    if (CoreApi::Bits() == 64) {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 8,
            .table_ = 16,
            .kind_ = 24,
        };
    } else {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 4,
            .table_ = 8,
            .kind_ = 12,
        };
    }
}

void IndirectReferenceTable::Init29() {
    if (CoreApi::Bits() == 64) {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 8,
            .table_ = 80,
            .kind_ = 88,
        };
    } else {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 4,
            .table_ = 44,
            .kind_ = 48,
        };
    }
}

void IndirectReferenceTable::Init34() {
    if (CoreApi::Bits() == 64) {
        __IndirectReferenceTable_offset__ = {
            .table_mem_map_ = 0,
            .table_ = 72,
            .kind_ = 80,
            .top_index_ = 88,
        };
    } else {
        __IndirectReferenceTable_offset__ = {
            .table_mem_map_ = 0,
            .table_ = 40,
            .kind_ = 44,
            .top_index_ = 48,
        };
    }
}

std::string IndirectReferenceTable::GetDescriptor(IndirectRefKind kind) {
    if (kind == IndirectRefKind::kLocal)
        return "JNI_LOCAL";
    else if (kind == IndirectRefKind::kGlobal)
        return "JNI_GLOBAL";
    else if (kind == art::IndirectRefKind::kWeakGlobal)
        return "JNI_WEAK_GLOBAL";
    return "JNI_UNK";
}

uint32_t IndirectReferenceTable::DecodeIndex(uint64_t uref) {
    if (Android::Sdk() >= Android::T) {
        return static_cast<uint32_t>((uref >> kKindBits) >> kIRTSerialBits);
    } else if (Android::Sdk() >= Android::O) {
        return static_cast<uint32_t>((uref >> kKindBits) >> kSerialBits);
    } else {
        return (uref >> 2) & 0xffff;
    }
}

mirror::Object IndirectReferenceTable::DecodeReference(uint32_t idx) {
    mirror::Object object = 0x0;
    uint64_t top_index_ = 0x0;

    if (Android::Sdk() >= Android::T) {
        top_index_ = top_index();
    } else if (Android::Sdk() >= Android::O) {
        top_index_ = segment_state();
    } else {
        top_index_ = segment_state() & 0xFFFF;
    }

    if (idx < top_index_) {
        IrtEntry entry = table();
        entry.MovePtr(SIZEOF(IrtEntry) * idx);
        api::MemoryRef ref = entry.references();
        if (Android::Sdk() < Android::T) {
            object = ref.value32Of(entry.serial() * sizeof(uint32_t));
        } else {
            object = ref.value32Of();
        }
    }
    return object;
}

uint64_t IndirectReferenceTable::EncodeIndex(uint32_t table_index) {
    if (Android::Sdk() >= Android::T) {
        return (static_cast<uint64_t>(table_index) << kKindBits << kIRTSerialBits);
    } else if (Android::Sdk() >= Android::O) {
        return (static_cast<uint64_t>(table_index) << kKindBits << kSerialBits);
    } else {
        return (static_cast<uint64_t>(table_index) << 2);
    }
}

uint64_t IndirectReferenceTable::EncodeSerial(uint32_t serial) {
    if (Android::Sdk() >= Android::O)
        return serial << kKindBits;
    else
        return serial << 20;
}

uint64_t IndirectReferenceTable::EncodeIndirectRefKind(IndirectRefKind kind) {
    return static_cast<uint64_t>(kind);
}

uint64_t IndirectReferenceTable::EncodeIndirectRef(uint32_t table_index, uint32_t serial) {
    return EncodeIndex(table_index) | EncodeSerial(serial) | EncodeIndirectRefKind(static_cast<IndirectRefKind>(kind()));
}

void IndirectReferenceTable::Walk(std::function<bool (mirror::Object&)> fn) {
    auto callback = [&](art::mirror::Object& object, uint64_t iref) -> bool {
        fn(object);
        return false;
    };
    Walk(callback);
}

void IndirectReferenceTable::Walk(std::function<bool (mirror::Object&, uint64_t)> fn) {
    mirror::Object object = 0x0;
    uint32_t top_index_ = 0x0;

    if (Android::Sdk() >= Android::T) {
        top_index_ = top_index();
    } else if (Android::Sdk() >= Android::O) {
        top_index_ = segment_state();
    } else {
        top_index_ = segment_state() & 0xFFFF;
    }

    for (int idx = 0; idx < top_index_; ++idx) {
        IrtEntry entry = table();
        entry.MovePtr(SIZEOF(IrtEntry) * idx);
        api::MemoryRef ref = entry.references();
        if (Android::Sdk() < Android::T) {
            object = ref.value32Of(entry.serial() * sizeof(uint32_t));
        } else {
            object = ref.value32Of();
        }
        if (object.Ptr() && object.IsValid())
            fn(object, EncodeIndirectRef(idx, entry.serial()));
    }
}

} // namespace art
