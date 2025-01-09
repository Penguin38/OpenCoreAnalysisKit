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

#ifndef ANDROID_ART_RUNTIME_INDIRECT_REFERENCE_TABLE_H_
#define ANDROID_ART_RUNTIME_INDIRECT_REFERENCE_TABLE_H_

#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include <functional>

struct IrtEntry_OffsetTable {
    uint32_t serial_;
    uint32_t references_;
};

struct IrtEntry_SizeTable {
    uint32_t THIS;
};

extern struct IrtEntry_OffsetTable __IrtEntry_offset__;
extern struct IrtEntry_SizeTable __IrtEntry_size__;

struct IndirectReferenceTable_OffsetTable {
    uint32_t segment_state_;
    uint32_t table_mem_map_;
    uint32_t table_;
    uint32_t kind_;
    uint32_t top_index_;
};

extern struct IndirectReferenceTable_OffsetTable __IndirectReferenceTable_offset__;

namespace art {

enum IndirectRefKind {
    kJniTransitionOrInvalid = 0,  // <<JNI transition frame reference or invalid reference>>
    kLocal                  = 1,  // <<local reference>>
    kGlobal                 = 2,  // <<global reference>>
    kWeakGlobal             = 3,  // <<weak global reference>>
    kLastKind               = kWeakGlobal
};

static constexpr uint32_t kSerialBits        = 0x2;
static constexpr uint32_t kKindBits          = 0x2;
static constexpr uint32_t kKindMask          = (1u << kKindBits) - 1;

// 33+
static constexpr uint32_t kIRTSerialBits     = 0x3;

class IrtEntry : public api::MemoryRef {
public:
    IrtEntry(uint64_t v) : api::MemoryRef(v) {}
    IrtEntry(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    IrtEntry(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    IrtEntry(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init23();
    static void Init33();
    inline uint32_t serial() { return value32Of(OFFSET(IrtEntry, serial_)); }
    inline uint64_t references() { return Ptr() + OFFSET(IrtEntry, references_); }
};

class IndirectReferenceTable : public api::MemoryRef {
public:
    IndirectReferenceTable(uint64_t v) : api::MemoryRef(v) {}
    IndirectReferenceTable(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    IndirectReferenceTable(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    IndirectReferenceTable(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static void Init23();
    static void Init29();
    static void Init34();
    inline uint32_t segment_state() { return value32Of(OFFSET(IndirectReferenceTable, segment_state_)); }
    inline uint64_t table_mem_map() { return Ptr() + OFFSET(IndirectReferenceTable, table_mem_map_); }
    inline uint64_t table_mem_map_lv28() { return VALUEOF(IndirectReferenceTable, table_mem_map_); }
    inline uint64_t table() { return VALUEOF(IndirectReferenceTable, table_); }
    inline uint64_t top_index() { return VALUEOF(IndirectReferenceTable, top_index_); }
    inline uint32_t kind() { return value32Of(OFFSET(IndirectReferenceTable, kind_)); }

    static inline IndirectRefKind DecodeIndirectRefKind(uint64_t uref) {
        return static_cast<IndirectRefKind>(uref & kKindMask);
    }

    uint64_t EncodeIndex(uint32_t table_index);
    uint64_t EncodeSerial(uint32_t serial);
    uint64_t EncodeIndirectRefKind(IndirectRefKind kind);
    uint64_t EncodeIndirectRef(uint32_t table_index, uint32_t serial);

    static std::string GetDescriptor(IndirectRefKind kind);
    static uint32_t DecodeIndex(uint64_t uref);
    mirror::Object DecodeReference(uint32_t idx);
    void Walk(std::function<bool (mirror::Object& object)> fn);
    void Walk(std::function<bool (mirror::Object& object, uint64_t iref)> fn);
};

} // namespace art

#endif  // ANDROID_ART_RUNTIME_INDIRECT_REFERENCE_TABLE_H_
