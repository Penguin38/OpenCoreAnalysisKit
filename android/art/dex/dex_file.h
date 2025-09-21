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

#ifndef ANDROID_ART_DEX_DEX_FILE_H_
#define ANDROID_ART_DEX_DEX_FILE_H_

#include "api/memory_ref.h"
#include "dex/dex_file_types.h"
#include "dex/dex_file_structs.h"
#include "runtime/oat/oat_file.h"
#include "cxx/string.h"

struct DexFile_OffsetTable {
    uint32_t begin_;
    uint32_t size_;
    uint32_t data_begin_;
    uint32_t data_size_;
    uint32_t location_;
    uint32_t location_checksum_;
    uint32_t header_;
    uint32_t string_ids_;
    uint32_t type_ids_;
    uint32_t field_ids_;
    uint32_t method_ids_;
    uint32_t proto_ids_;
    uint32_t class_defs_;
    uint32_t method_handles_;
    uint32_t num_method_handles_;
    uint32_t call_site_ids_;
    uint32_t num_call_site_ids_;
    uint32_t hiddenapi_class_data_;
    uint32_t oat_dex_file_;
    uint32_t container_;
    uint32_t is_compact_dex_;
};

struct DexFile_SizeTable {
    uint32_t THIS;
};

extern struct DexFile_OffsetTable __DexFile_offset__;
extern struct DexFile_SizeTable __DexFile_size__;

namespace art {

class DexFile : public api::MemoryRef {
public:
    DexFile(uint64_t v) : api::MemoryRef(v) {}
    DexFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    DexFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(DexFile& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(DexFile& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    static void Init23();
    static void Init24();
    static void Init26();
    static void Init28();
    static void Init29();
    static void Init34();
    inline uint64_t data_begin() { return VALUEOF(DexFile, data_begin_); }
    inline uint64_t data_size() { return VALUEOF(DexFile, data_size_); }
    inline uint64_t location() { return Ptr() + OFFSET(DexFile, location_); }
    inline uint32_t location_checksum() { return value32Of(OFFSET(DexFile, location_checksum_)); }
    inline uint64_t type_ids() { return VALUEOF(DexFile, type_ids_); }
    inline uint64_t string_ids() { return VALUEOF(DexFile, string_ids_); }
    inline uint64_t field_ids() { return VALUEOF(DexFile, field_ids_); }
    inline uint64_t oat_dex_file() { return VALUEOF(DexFile, oat_dex_file_); }
    inline uint64_t method_ids() { return VALUEOF(DexFile, method_ids_); }
    inline uint64_t proto_ids() { return VALUEOF(DexFile, proto_ids_); }
    inline uint8_t is_compact_dex() { return value8Of(OFFSET(DexFile, is_compact_dex_)); }

    inline api::MemoryRef& DataBegin() { return QUICK_CACHE(data_begin); }
    cxx::string GetLocation();
    dex::TypeId GetTypeId(dex::TypeIndex idx);
    dex::MethodId GetMethodId(uint32_t idx);
    dex::ProtoId GetMethodPrototype(dex::MethodId& method_id);
    dex::ProtoId GetProtoId(dex::ProtoIndex idx);
    inline const char* GetTypeDescriptor(dex::TypeId& type_id) { return GetTypeDescriptor(type_id, "L<invalid-class>;"); }
    const char* GetTypeDescriptor(dex::TypeId& type_id, const char* def);
    inline const char* StringDataByIdx(dex::StringIndex idx) { return StringDataByIdx(idx, "<unknown>"); }
    const char* StringDataByIdx(dex::StringIndex idx, const char* def);
    const char* StringDataAndUtf16LengthByIdx(dex::StringIndex idx, uint32_t* utf16_length, const char* def);
    dex::StringId GetStringId(dex::StringIndex idx);
    const char* GetStringDataAndUtf16Length(dex::StringId& string_id, uint32_t* utf16_length, const char* def);
    dex::FieldId GetFieldId(uint32_t idx);
    inline const char* GetFieldTypeDescriptor(dex::FieldId& field_id) { return GetFieldTypeDescriptor(field_id, "B"); }
    const char* GetFieldTypeDescriptor(dex::FieldId& field_id, const char* def);
    inline const char* GetFieldDeclaringClassDescriptor(dex::FieldId& field_id) { return GetFieldDeclaringClassDescriptor(field_id, "L<invalid-class>;"); }
    const char* GetFieldDeclaringClassDescriptor(dex::FieldId& field_id, const char* def);
    inline const char* GetFieldName(dex::FieldId& field_id) { return GetFieldName(field_id, "<invalid-field>"); }
    const char* GetFieldName(dex::FieldId& field_id, const char* def);
    const char* GetMethodName(dex::MethodId& method_id);
    std::string GetMethodParametersDescriptor(dex::ProtoId& proto_id);
    std::string PrettyMethodParameters(dex::MethodId& method_id);
    inline const char* GetMethodDeclaringClassDescriptor(dex::MethodId& method_id) { return GetMethodDeclaringClassDescriptor(method_id, "L<invalid-class>;"); }
    const char* GetMethodDeclaringClassDescriptor(dex::MethodId& method_id, const char* def);
    inline const char* GetMethodReturnTypeDescriptor(dex::MethodId& method_id) { return GetMethodReturnTypeDescriptor(method_id, "V"); }
    const char* GetMethodReturnTypeDescriptor(dex::MethodId& method_id, const char* def);
    inline OatDexFile& GetOatDexFile() { return QUICK_CACHE(oat_dex_file); }
    inline bool IsCompactDexFile() { return !!is_compact_dex(); }
    inline bool IsStandardDexFile() { return !is_compact_dex(); }
    const char* GetMethodShorty(dex::MethodId& method_id, uint32_t* length);
    void dumpReason(uint64_t vaddr);
private:
    // quick memoryref cache
    DEFINE_QUICK_CACHE(api::MemoryRef, data_begin);
    DEFINE_QUICK_CACHE_COPY(dex::TypeId, type_ids, data_begin);
    DEFINE_QUICK_CACHE_COPY(dex::StringId, string_ids, data_begin);
    DEFINE_QUICK_CACHE_COPY(dex::FieldId, field_ids, data_begin);
    DEFINE_QUICK_CACHE_COPY(dex::MethodId, method_ids, data_begin)
    DEFINE_QUICK_CACHE_COPY(dex::ProtoId, proto_ids, data_begin);
    DEFINE_QUICK_CACHE(OatDexFile, oat_dex_file);
};

} // namespace art

#endif // ANDROID_ART_DEX_DEX_FILE_H_
