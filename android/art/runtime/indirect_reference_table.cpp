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

#include "logger/log.h"
#include "runtime/indirect_reference_table.h"
#include "android.h"

struct IrtEntry_OffsetTable __IrtEntry_offset__;
struct IrtEntry_SizeTable __IrtEntry_size__;
struct IndirectReferenceTable_OffsetTable __IndirectReferenceTable_offset__;

namespace art {

void IrtEntry::Init26() {
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

void IndirectReferenceTable::Init26() {
    if (CoreApi::Bits() == 64) {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 8,
            .table_ = 16,
        };
    } else {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 4,
            .table_ = 8,
        };
    }
}

void IndirectReferenceTable::Init29() {
    if (CoreApi::Bits() == 64) {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 8,
            .table_ = 80,
        };
    } else {
        __IndirectReferenceTable_offset__ = {
            .segment_state_ = 0,
            .table_mem_map_ = 4,
            .table_ = 44,
        };
    }
}

void IndirectReferenceTable::Init34() {
    if (CoreApi::Bits() == 64) {
        __IndirectReferenceTable_offset__ = {
            .table_mem_map_ = 0,
            .table_ = 72,
            .top_index_ = 88,
        };
    } else {
        __IndirectReferenceTable_offset__ = {
            .table_mem_map_ = 0,
            .table_ = 40,
            .top_index_ = 48,
        };
    }
}

uint32_t IndirectReferenceTable::DecodeIndex(uint64_t uref) {
    if (Android::Sdk() < Android::TIRAMISU) {
        return static_cast<uint32_t>((uref >> kKindBits) >> kSerialBits);
    } else {
        return static_cast<uint32_t>((uref >> kKindBits) >> kIRTSerialBits);
    }
}

mirror::Object IndirectReferenceTable::DecodeReference(uint32_t idx) {
    mirror::Object object = 0x0;
    uint64_t top_index_ = 0x0;

    if (Android::Sdk() < Android::TIRAMISU) {
        top_index_ = segment_state();
    } else {
        top_index_ = top_index();
    }

    if (idx < top_index_) {
        IrtEntry entry = table();
        entry.MovePtr(SIZEOF(IrtEntry) * idx);
        api::MemoryRef ref = entry.references();
        if (Android::Sdk() < Android::TIRAMISU) {
            object = ref.value32Of(entry.serial() * sizeof(uint32_t));
        } else {
            object = ref.value32Of();
        }
    }
    return object;
}

} // namespace art
