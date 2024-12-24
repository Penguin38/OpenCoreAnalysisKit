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

#include "api/core.h"
#include "android.h"
#include "logger/log.h"
#include "dex/dex_file.h"
#include "base/leb128.h"
#include "dex/descriptors_names.h"

struct DexFile_OffsetTable __DexFile_offset__;
struct DexFile_SizeTable __DexFile_size__;

namespace art {

void DexFile::Init() {
    Android::RegisterSdkListener(Android::M, art::DexFile::Init23);
    Android::RegisterSdkListener(Android::N, art::DexFile::Init24);
    Android::RegisterSdkListener(Android::O, art::DexFile::Init26);
    Android::RegisterSdkListener(Android::P, art::DexFile::Init28);
    Android::RegisterSdkListener(Android::Q, art::DexFile::Init29);
    Android::RegisterSdkListener(Android::U, art::DexFile::Init34);
}

void DexFile::Init23() {
    if (CoreApi::Bits() == 64) {
        __DexFile_offset__ = {
            .begin_ = 8,
            .size_ = 16,
            .data_begin_ = 8,
            .data_size_ = 16,
            .location_ = 24,
            .location_checksum_ = 48,
            .header_ = 64,
            .string_ids_ = 72,
            .type_ids_ = 80,
            .field_ids_ = 88,
            .method_ids_ = 96,
            .proto_ids_ = 104,
            .class_defs_ = 112,
            .method_handles_ = 120,
            .num_method_handles_ = 128,
            .call_site_ids_ = 136,
            .num_call_site_ids_ = 144,
            .hiddenapi_class_data_ = 0,
            .oat_dex_file_ = 152,
            .container_ = 0,
            .is_compact_dex_ = 0,
        };

         __DexFile_size__ = {
            .THIS = 160,
            .begin_ = 8,
            .size_ = 8,
            .data_begin_ = 8,
            .data_size_ = 8,
            .location_ = 24,
            .location_checksum_ = 8,
            .header_ = 8,
            .string_ids_ = 8,
            .type_ids_ = 8,
            .field_ids_ = 8,
            .method_ids_ = 8,
            .proto_ids_ = 8,
            .class_defs_ = 8,
            .method_handles_ = 8,
            .num_method_handles_ = 8,
            .call_site_ids_ = 8,
            .num_call_site_ids_ = 8,
            .hiddenapi_class_data_ = 8,
            .oat_dex_file_ = 8,
        };
    } else {
        // TODO
    }
}

void DexFile::Init24() {
    if (CoreApi::Bits() == 64) {
        __DexFile_offset__ = {
            .begin_ = 8,
            .size_ = 16,
            .data_begin_ = 8,
            .data_size_ = 16,
            .location_ = 24,
            .location_checksum_ = 48,
            .header_ = 64,
            .string_ids_ = 72,
            .type_ids_ = 80,
            .field_ids_ = 88,
            .method_ids_ = 96,
            .proto_ids_ = 104,
            .class_defs_ = 112,
            .oat_dex_file_ = 120,
        };

         __DexFile_size__ = {
            .THIS = 128,
            .begin_ = 8,
            .size_ = 8,
            .data_begin_ = 8,
            .data_size_ = 8,
            .location_ = 24,
            .location_checksum_ = 8,
            .header_ = 8,
            .string_ids_ = 8,
            .type_ids_ = 8,
            .field_ids_ = 8,
            .method_ids_ = 8,
            .proto_ids_ = 8,
            .class_defs_ = 8,
            .oat_dex_file_ = 8,
        };
    } else {
        // TODO
    }
}


void DexFile::Init26() {
    if (CoreApi::Bits() == 64) {
        __DexFile_offset__ = {
            .begin_ = 8,
            .size_ = 16,
            .data_begin_ = 8,
            .data_size_ = 16,
            .location_ = 24,
            .location_checksum_ = 48,
            .header_ = 64,
            .string_ids_ = 72,
            .type_ids_ = 80,
            .field_ids_ = 88,
            .method_ids_ = 96,
            .proto_ids_ = 104,
            .class_defs_ = 112,
            .method_handles_ = 120,
            .num_method_handles_ = 128,
            .call_site_ids_ = 136,
            .num_call_site_ids_ = 144,
            .hiddenapi_class_data_ = 0,
            .oat_dex_file_ = 152,
            .container_ = 0,
            .is_compact_dex_ = 0,
        };

         __DexFile_size__ = {
            .THIS = 160,
            .begin_ = 8,
            .size_ = 8,
            .data_begin_ = 8,
            .data_size_ = 8,
            .location_ = 24,
            .location_checksum_ = 8,
            .header_ = 8,
            .string_ids_ = 8,
            .type_ids_ = 8,
            .field_ids_ = 8,
            .method_ids_ = 8,
            .proto_ids_ = 8,
            .class_defs_ = 8,
            .method_handles_ = 8,
            .num_method_handles_ = 8,
            .call_site_ids_ = 8,
            .num_call_site_ids_ = 8,
            .hiddenapi_class_data_ = 8,
            .oat_dex_file_ = 8,
        };
    } else {
        __DexFile_offset__ = {
            .begin_ = 4,
            .size_ = 8,
            .data_begin_ = 4,
            .data_size_ = 8,
            .location_ = 12,
            .location_checksum_ = 24,
            .header_ = 32,
            .string_ids_ = 36,
            .type_ids_ = 40,
            .field_ids_ = 44,
            .method_ids_ = 48,
            .proto_ids_ = 52,
            .class_defs_ = 56,
            .method_handles_ = 60,
            .num_method_handles_ = 64,
            .call_site_ids_ = 68,
            .num_call_site_ids_ = 72,
            .hiddenapi_class_data_ = 0,
            .oat_dex_file_ = 76,
            .container_ = 0,
            .is_compact_dex_ = 0,
        };

         __DexFile_size__ = {
            .THIS = 80,
            .begin_ = 4,
            .size_ = 4,
            .data_begin_ = 4,
            .data_size_ = 4,
            .location_ = 12,
            .location_checksum_ = 4,
            .header_ = 4,
            .string_ids_ = 4,
            .type_ids_ = 4,
            .field_ids_ = 4,
            .method_ids_ = 4,
            .proto_ids_ = 4,
            .class_defs_ = 4,
            .method_handles_ = 4,
            .num_method_handles_ = 4,
            .call_site_ids_ = 4,
            .num_call_site_ids_ = 4,
            .hiddenapi_class_data_ = 4,
            .oat_dex_file_ = 4,
        };
    }
}

void DexFile::Init28() {
    if (CoreApi::Bits() == 64) {
        __DexFile_offset__ = {
            .begin_ = 8,
            .size_ = 16,
            .data_begin_ = 24,
            .data_size_ = 32,
            .location_ = 40,
            .location_checksum_ = 64,
            .header_ = 72,
            .string_ids_ = 80,
            .type_ids_ = 88,
            .field_ids_ = 96,
            .method_ids_ = 104,
            .proto_ids_ = 112,
            .class_defs_ = 120,
            .method_handles_ = 128,
            .num_method_handles_ = 136,
            .call_site_ids_ = 144,
            .num_call_site_ids_ = 152,
            .hiddenapi_class_data_ = 0,
            .oat_dex_file_ = 160,
            .container_ = 168,
            .is_compact_dex_ = 176,
        };

         __DexFile_size__ = {
            .THIS = 184,
            .begin_ = 8,
            .size_ = 8,
            .data_begin_ = 8,
            .data_size_ = 8,
            .location_ = 24,
            .location_checksum_ = 8,
            .header_ = 8,
            .string_ids_ = 8,
            .type_ids_ = 8,
            .field_ids_ = 8,
            .method_ids_ = 8,
            .proto_ids_ = 8,
            .class_defs_ = 8,
            .method_handles_ = 8,
            .num_method_handles_ = 8,
            .call_site_ids_ = 8,
            .num_call_site_ids_ = 8,
            .hiddenapi_class_data_ = 8,
            .oat_dex_file_ = 8,
            .container_ = 8,
            .is_compact_dex_ = 1,
        };
    } else {
        __DexFile_offset__ = {
            .begin_ = 4,
            .size_ = 8,
            .data_begin_ = 12,
            .data_size_ = 16,
            .location_ = 20,
            .location_checksum_ = 32,
            .header_ = 36,
            .string_ids_ = 40,
            .type_ids_ = 44,
            .field_ids_ = 48,
            .method_ids_ = 52,
            .proto_ids_ = 56,
            .class_defs_ = 60,
            .method_handles_ = 64,
            .num_method_handles_ = 68,
            .call_site_ids_ = 72,
            .num_call_site_ids_ = 76,
            .hiddenapi_class_data_ = 0,
            .oat_dex_file_ = 80,
            .container_ = 84,
            .is_compact_dex_ = 88,
        };

        __DexFile_size__ = {
            .THIS = 92,
            .begin_ = 4,
            .size_ = 4,
            .data_begin_ = 4,
            .data_size_ = 4,
            .location_ = 12,
            .location_checksum_ = 4,
            .header_ = 4,
            .string_ids_ = 4,
            .type_ids_ = 4,
            .field_ids_ = 4,
            .method_ids_ = 4,
            .proto_ids_ = 4,
            .class_defs_ = 4,
            .method_handles_ = 4,
            .num_method_handles_ = 4,
            .call_site_ids_ = 4,
            .num_call_site_ids_ = 4,
            .hiddenapi_class_data_ = 4,
            .oat_dex_file_ = 4,
            .container_ = 4,
            .is_compact_dex_ = 1,
        };
    }
}


void DexFile::Init29() {
    // 29 ~ 33
    if (CoreApi::Bits() == 64) {
        __DexFile_offset__ = {
            .begin_ = 8,
            .size_ = 16,
            .data_begin_ = 24,
            .data_size_ = 32,
            .location_ = 40,
            .location_checksum_ = 64,
            .header_ = 72,
            .string_ids_ = 80,
            .type_ids_ = 88,
            .field_ids_ = 96,
            .method_ids_ = 104,
            .proto_ids_ = 112,
            .class_defs_ = 120,
            .method_handles_ = 128,
            .num_method_handles_ = 136,
            .call_site_ids_ = 144,
            .num_call_site_ids_ = 152,
            .hiddenapi_class_data_ = 160,
            .oat_dex_file_ = 168,
            .container_ = 176,
            .is_compact_dex_ = 184,
        };

         __DexFile_size__ = {
            .THIS = 192,
            .begin_ = 8,
            .size_ = 8,
            .data_begin_ = 8,
            .data_size_ = 8,
            .location_ = 24,
            .location_checksum_ = 8,
            .header_ = 8,
            .string_ids_ = 8,
            .type_ids_ = 8,
            .field_ids_ = 8,
            .method_ids_ = 8,
            .proto_ids_ = 8,
            .class_defs_ = 8,
            .method_handles_ = 8,
            .num_method_handles_ = 8,
            .call_site_ids_ = 8,
            .num_call_site_ids_ = 8,
            .hiddenapi_class_data_ = 8,
            .oat_dex_file_ = 8,
            .container_ = 8,
            .is_compact_dex_ = 1,
        };
    } else {
        __DexFile_offset__ = {
            .begin_ = 4,
            .size_ = 8,
            .data_begin_ = 12,
            .data_size_ = 16,
            .location_ = 20,
            .location_checksum_ = 32,
            .header_ = 36,
            .string_ids_ = 40,
            .type_ids_ = 44,
            .field_ids_ = 48,
            .method_ids_ = 52,
            .proto_ids_ = 56,
            .class_defs_ = 60,
            .method_handles_ = 64,
            .num_method_handles_ = 68,
            .call_site_ids_ = 72,
            .num_call_site_ids_ = 76,
            .hiddenapi_class_data_ = 80,
            .oat_dex_file_ = 84,
            .container_ = 88,
            .is_compact_dex_ = 92,
        };

        __DexFile_size__ = {
            .THIS = 100,
            .begin_ = 4,
            .size_ = 4,
            .data_begin_ = 4,
            .data_size_ = 4,
            .location_ = 12,
            .location_checksum_ = 4,
            .header_ = 4,
            .string_ids_ = 4,
            .type_ids_ = 4,
            .field_ids_ = 4,
            .method_ids_ = 4,
            .proto_ids_ = 4,
            .class_defs_ = 4,
            .method_handles_ = 4,
            .num_method_handles_ = 4,
            .call_site_ids_ = 4,
            .num_call_site_ids_ = 4,
            .hiddenapi_class_data_ = 4,
            .oat_dex_file_ = 4,
            .container_ = 4,
            .is_compact_dex_ = 1,
        };
    }
}

void DexFile::Init34() {
    // 34 ~ x
    if (CoreApi::Bits() == 64) {
        __DexFile_offset__.is_compact_dex_ = 192;

        __DexFile_size__.THIS = 200;
        __DexFile_size__.container_ = 16;
    } else {
        __DexFile_offset__.is_compact_dex_ = 96;

        __DexFile_size__.THIS = 100;
        __DexFile_size__.container_ = 8;
    }
}

cxx::string DexFile::GetLocation() {
    cxx::string location_(location(), this);
    return location_;
}

dex::TypeId DexFile::GetTypeId(dex::TypeIndex idx) {
    dex::TypeId type_id(QUICK_CACHE(type_ids).Ptr() + SIZEOF(TypeId) * idx.index_, QUICK_CACHE(type_ids));
    return type_id;
}

dex::MethodId DexFile::GetMethodId(uint32_t idx) {
    dex::MethodId method_id(QUICK_CACHE(method_ids).Ptr() + SIZEOF(MethodId) * idx, QUICK_CACHE(method_ids));
    return method_id;
}

dex::ProtoId DexFile::GetMethodPrototype(dex::MethodId& method_id) {
    dex::ProtoIndex idx(method_id.proto_idx());
    return GetProtoId(idx);
}

dex::ProtoId DexFile::GetProtoId(dex::ProtoIndex idx) {
    dex::ProtoId proto_id(QUICK_CACHE(proto_ids).Ptr() + SIZEOF(ProtoId) * idx.index_, QUICK_CACHE(proto_ids));
    return proto_id;
}

const char* DexFile::GetTypeDescriptor(dex::TypeId& type_id, const char* def) {
    if (!type_id.IsValid()) {
        dumpReason(type_id.Ptr());
        return def;
    }
    dex::StringIndex idx(type_id.descriptor_idx());
    return StringDataByIdx(idx);
}

const char* DexFile::StringDataByIdx(dex::StringIndex idx) {
    uint32_t unicode_length;
    return StringDataAndUtf16LengthByIdx(idx, &unicode_length);
}

const char* DexFile::StringDataAndUtf16LengthByIdx(dex::StringIndex idx, uint32_t* utf16_length) {
    if (!idx.IsValid()) {
        *utf16_length = 0;
        return nullptr;
    }
    dex::StringId string_id = GetStringId(idx);
    return GetStringDataAndUtf16Length(string_id, utf16_length);
}

dex::StringId DexFile::GetStringId(dex::StringIndex idx) {
    dex::StringId string_id(QUICK_CACHE(string_ids).Ptr() + SIZEOF(StringId) * idx.index_, QUICK_CACHE(string_ids));
    return string_id;
}

const char* DexFile::GetStringDataAndUtf16Length(dex::StringId& string_id, uint32_t* utf16_length) {
    api::MemoryRef ref(DataBegin().Ptr() + string_id.string_data_off(), string_id);
    const uint8_t* ptr = reinterpret_cast<const uint8_t *>(ref.Real());
    *utf16_length = DecodeUnsignedLeb128(&ptr);
    return reinterpret_cast<const char*>(ptr);
}

dex::FieldId DexFile::GetFieldId(uint32_t idx) {
    dex::FieldId field_id(QUICK_CACHE(field_ids).Ptr() + SIZEOF(FieldId) * idx, QUICK_CACHE(field_ids));
    return field_id;
}

const char* DexFile::GetFieldTypeDescriptor(dex::FieldId& field_id, const char* def) {
    if (!field_id.IsValid()) {
        dumpReason(field_id.Ptr());
        return def;
    }
    dex::TypeIndex idx(field_id.type_idx());
    dex::TypeId type_id = GetTypeId(idx);
    return GetTypeDescriptor(type_id, def);
}

const char* DexFile::GetFieldDeclaringClassDescriptor(dex::FieldId& field_id, const char* def) {
    if (!field_id.IsValid()) {
        dumpReason(field_id.Ptr());
        return def;
    }
    dex::TypeIndex class_idx(field_id.class_idx());
    dex::TypeId class_id = GetTypeId(class_idx);
    return GetTypeDescriptor(class_id, def);
}

const char* DexFile::GetFieldName(dex::FieldId& field_id, const char* def) {
    if (!field_id.IsValid()) {
        dumpReason(field_id.Ptr());
        return def;
    }
    dex::StringIndex idx(field_id.name_idx());
    return StringDataByIdx(idx);
}

const char* DexFile::GetMethodName(dex::MethodId& method_id) {
    if (!method_id.IsValid()) {
        dumpReason(method_id.Ptr());
        return "<unknown>";
    }
    dex::StringIndex idx(method_id.name_idx());
    return StringDataByIdx(idx);
}

std::string DexFile::GetMethodParametersDescriptor(dex::ProtoId& proto_id) {
    if (!proto_id.IsValid()) {
        dumpReason(proto_id.Ptr());
        return "(...)";
    }

    std::string result;
    if (proto_id.parameters_off()) {
        dex::TypeList list = data_begin() + proto_id.parameters_off();
        api::MemoryRef ref = list.list();
        result.append("(");
        for (uint32_t i = 0; i < list.size(); ++i) {
            dex::TypeIndex idx(ref.value16Of(i * SIZEOF(TypeItem)));
            dex::TypeId type_id = GetTypeId(idx);
            result.append(GetTypeDescriptor(type_id, "V"));
        }
        result.append(")");
        return result;
    }
    return "()";
}

std::string DexFile::PrettyMethodParameters(dex::MethodId& method_id) {
    if (!method_id.IsValid()) {
        dumpReason(method_id.Ptr());
        return "(...)";
    }

    std::string result;
    dex::ProtoId proto_id = GetMethodPrototype(method_id);
    if (proto_id.parameters_off()) {
        dex::TypeList list = data_begin() + proto_id.parameters_off();
        api::MemoryRef ref = list.list();
        result.append("(");
        for (uint32_t i = 0; i < list.size(); ++i) {
            if (i > 0) {
                result.append(", ");
            }
            dex::TypeIndex idx(ref.value16Of(i * SIZEOF(TypeItem)));
            dex::TypeId type_id = GetTypeId(idx);
            std::string tmp;
            AppendPrettyDescriptor(GetTypeDescriptor(type_id, "V"), &tmp);
            result.append(tmp);
        }
        result.append(")");
        return result;
    }
    return "()";
}

const char* DexFile::GetMethodDeclaringClassDescriptor(dex::MethodId& method_id, const char* def) {
    if (!method_id.IsValid()) {
        dumpReason(method_id.Ptr());
        return def;
    }
    dex::TypeIndex class_idx(method_id.class_idx());
    dex::TypeId class_id = GetTypeId(class_idx);
    return GetTypeDescriptor(class_id, def);
}

const char* DexFile::GetMethodReturnTypeDescriptor(dex::MethodId& method_id, const char* def) {
    if (!method_id.IsValid()) {
        dumpReason(method_id.Ptr());
        return def;
    }
    dex::ProtoId proto_id = GetMethodPrototype(method_id);
    dex::TypeIndex return_type_idx(proto_id.return_type_idx());
    dex::TypeId return_type_id = GetTypeId(return_type_idx);
    return GetTypeDescriptor(return_type_id);
}

const char* DexFile::GetMethodShorty(dex::MethodId& method_id, uint32_t* length) {
    dex::ProtoIndex idx(method_id.proto_idx());
    dex::ProtoId pid = GetProtoId(idx);
    dex::StringIndex sidx(pid.shorty_idx());
    return StringDataAndUtf16LengthByIdx(sidx, length);
}

void DexFile::dumpReason(uint64_t vaddr) {
    if (Logger::IsDebug()) {
        File* file = CoreApi::FindFile(vaddr);
        if (file) {
            LOGD("[%" PRIx64 ", %" PRIx64 ") %08" PRIx64 " %s [EMPTY]\n", file->begin(), file->end(),
                    file->offset(), file->name().c_str());
        } else {
            LoadBlock* block = CoreApi::FindLoadBlock(vaddr, false);
            if (block) {
                LOGD("[%" PRIx64 ", %" PRIx64 ") %s [EMPTY]\n", block->vaddr(), block->vaddr() + block->size(), GetLocation().c_str());
            }
        }
    }
}

} // namespace art
