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
#include "dex/dex_file.h"
#include "base/leb128.h"

struct DexFile_OffsetTable __DexFile_offset__;
struct DexFile_SizeTable __DexFile_size__;

namespace art {

void DexFile::Init() {
    // 30 ~ 33
    if (CoreApi::GetPointSize() == 64) {
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
    if (CoreApi::GetPointSize() == 64) {
        __DexFile_offset__.is_compact_dex_ = 192;

        __DexFile_size__.THIS = 200;
        __DexFile_size__.container_ = 16;
    } else {
        __DexFile_offset__.is_compact_dex_ = 96;

        __DexFile_size__.THIS = 100;
        __DexFile_size__.container_ = 8;
    }
}

uint8_t* DexFile::DataBegin() {
    return reinterpret_cast<uint8_t *>(data_begin());
}

dex::TypeId DexFile::GetTypeId(dex::TypeIndex idx) {
    if (!type_ids_cache.Ptr()) {
        type_ids_cache = type_ids();
        type_ids_cache.Prepare(false);
    }
    dex::TypeId type_id(type_ids() + SIZEOF(TypeId) * idx.index_, type_ids_cache);
    return type_id;
}

const char * DexFile::GetTypeDescriptor(dex::TypeId& type_id) {
    dex::StringIndex idx(type_id.descriptor_idx());
    return StringDataByIdx(idx);
}

const char * DexFile::StringDataByIdx(dex::StringIndex idx) {
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
    if (!string_ids_cache.Ptr()) {
        string_ids_cache = string_ids();
        string_ids_cache.Prepare(false);
    }
    dex::StringId string_id(string_ids() + SIZEOF(StringId) * idx.index_, string_ids_cache);
    return string_id;
}

const char* DexFile::GetStringDataAndUtf16Length(dex::StringId& string_id, uint32_t* utf16_length) {
    api::MemoryRef ref(DataBegin() + string_id.string_data_off(), string_id);
    const uint8_t* ptr = reinterpret_cast<const uint8_t *>(ref.Real());
    *utf16_length = DecodeUnsignedLeb128(&ptr);
    return reinterpret_cast<const char*>(ptr);
}

dex::FieldId DexFile::GetFieldId(uint32_t idx) {
    if (!field_ids_cache.Ptr()) {
        field_ids_cache = field_ids();
        field_ids_cache.Prepare(false);
    }
    dex::FieldId field_id(field_ids() + SIZEOF(FieldId) * idx, field_ids_cache);
    return field_id;
}

const char* DexFile::GetFieldTypeDescriptor(dex::FieldId& field_id) {
    dex::TypeIndex idx(field_id.type_idx());
    dex::TypeId type_id = GetTypeId(idx);
    return GetTypeDescriptor(type_id);
}

const char* DexFile::GetFieldName(dex::FieldId& field_id) {
    dex::StringIndex idx(field_id.name_idx());
    return StringDataByIdx(idx);
}

} // namespace art
