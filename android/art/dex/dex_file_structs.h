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

#ifndef ANDROID_ART_DEX_DEX_FILE_STRUCTS_H_
#define ANDROID_ART_DEX_DEX_FILE_STRUCTS_H_

#include "api/memory_ref.h"

struct TypeId_OffsetTable {
    uint32_t descriptor_idx_;
};

struct TypeId_SizeTable {
    uint32_t THIS;
};

extern struct TypeId_OffsetTable __TypeId_offset__;
extern struct TypeId_SizeTable __TypeId_size__;

struct StringId_OffsetTable {
    uint32_t string_data_off_;
};

struct StringId_SizeTable {
    uint32_t THIS;
};

extern struct StringId_OffsetTable __StringId_offset__;
extern struct StringId_SizeTable __StringId_size__;

struct FieldId_OffsetTable {
    uint32_t class_idx_;
    uint32_t type_idx_;
    uint32_t name_idx_;
};

struct FieldId_SizeTable {
    uint32_t THIS;
};

extern struct FieldId_OffsetTable __FieldId_offset__;
extern struct FieldId_SizeTable __FieldId_size__;

struct MethodId_OffsetTable {
    uint32_t class_idx_;
    uint32_t proto_idx_;
    uint32_t name_idx_;
};

struct MethodId_SizeTable {
    uint32_t THIS;
};

extern struct MethodId_OffsetTable __MethodId_offset__;
extern struct MethodId_SizeTable __MethodId_size__;

struct ProtoId_OffsetTable {
    uint32_t shorty_idx_;
    uint32_t return_type_idx_;
    uint32_t pad_;
    uint32_t parameters_off_;
};

struct ProtoId_SizeTable {
    uint32_t THIS;
};

extern struct ProtoId_OffsetTable __ProtoId_offset__;
extern struct ProtoId_SizeTable __ProtoId_size__;

struct TypeList_OffsetTable {
    uint32_t size_;
    uint32_t list_;
};

struct TypeList_SizeTable {
    uint32_t THIS;
};

extern struct TypeList_OffsetTable __TypeList_offset__;
extern struct TypeList_SizeTable __TypeList_size__;

struct TypeItem_OffsetTable {
    uint32_t type_idx_;
};

struct TypeItem_SizeTable {
    uint32_t THIS;
};

extern struct TypeItem_OffsetTable __TypeItem_offset__;
extern struct TypeItem_SizeTable __TypeItem_size__;

namespace art {
namespace dex {

class CodeItem : public api::MemoryRef {
public:
    CodeItem(uint64_t v) : api::MemoryRef(v) {}
    CodeItem(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    CodeItem(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    CodeItem(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    uint16_t num_regs_;
    uint16_t out_regs_;
    uint32_t insns_count_;
    uint16_t ins_size_;
    uint16_t tries_size_;
    uint32_t code_offset_;
};

class TypeId : public api::MemoryRef {
public:
    TypeId(uint64_t v) : api::MemoryRef(v) {}
    TypeId(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    TypeId(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    TypeId(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(TypeId& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(TypeId& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t descriptor_idx() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(TypeId, descriptor_idx_)); }
};

class StringId : public api::MemoryRef {
public:
    StringId(uint64_t v) : api::MemoryRef(v) {}
    StringId(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    StringId(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    StringId(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(StringId& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(StringId& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t string_data_off() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(StringId, string_data_off_)); }
};

class FieldId : public api::MemoryRef {
public:
    FieldId(uint64_t v) : api::MemoryRef(v) {}
    FieldId(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    FieldId(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    FieldId(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(FieldId& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(FieldId& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint16_t class_idx() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(FieldId, class_idx_)); }
    inline uint16_t type_idx() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(FieldId, type_idx_)); }
    inline uint32_t name_idx() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(FieldId, name_idx_)); }
};

class MethodId : public api::MemoryRef {
public:
    MethodId(uint64_t v) : api::MemoryRef(v) {}
    MethodId(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    MethodId(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    MethodId(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(MethodId& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(MethodId& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint16_t class_idx() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(MethodId, class_idx_)); }
    inline uint16_t proto_idx() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(MethodId, proto_idx_)); }
    inline uint32_t name_idx() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(MethodId, name_idx_)); }
};

class ProtoId : public api::MemoryRef {
public:
    ProtoId(uint64_t v) : api::MemoryRef(v) {}
    ProtoId(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ProtoId(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ProtoId(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(ProtoId& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(ProtoId& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t shorty_idx() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(ProtoId, shorty_idx_)); }
    inline uint16_t return_type_idx() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(ProtoId, return_type_idx_)); }
    inline uint16_t pad() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(ProtoId, pad_)); }
    inline uint32_t parameters_off() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(ProtoId, parameters_off_)); }
};

class TypeList : public api::MemoryRef {
public:
    TypeList(uint64_t v) : api::MemoryRef(v) {}
    TypeList(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    TypeList(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    TypeList(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(TypeList& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(TypeList& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t size() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(TypeList, size_)); }
    inline uint64_t list() { return Ptr() + OFFSET(TypeList, list_); }
};

class TypeItem : public api::MemoryRef {
public:
    TypeItem(uint64_t v) : api::MemoryRef(v) {}
    TypeItem(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    TypeItem(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    TypeItem(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(TypeItem& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(TypeItem& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint16_t type_idx() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(FieldId, type_idx_)); }
};

} // namespace dex
} // namespace art

#endif  // ANDROID_ART_DEX_DEX_FILE_STRUCTS_H_
