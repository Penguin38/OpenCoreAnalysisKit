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

extern struct DexFile_OffsetTable __DexFile_offset__;
extern struct DexFile_SizeTable __DexFile_size__;

namespace art {

class DexFile : public api::MemoryRef {
public:
    DexFile(uint64_t v) : api::MemoryRef(v) {}
    DexFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    DexFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> DexFile(U *v) : api::MemoryRef(v) {}
    template<typename U> DexFile(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(DexFile& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(DexFile& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t data_begin() { return *reinterpret_cast<uint64_t *>(Real() + OFFSET(DexFile, data_begin_)) & CoreApi::GetPointMask(); }
    inline uint64_t type_ids() { return *reinterpret_cast<uint64_t *>(Real() + OFFSET(DexFile, type_ids_)) & CoreApi::GetPointMask(); }
    inline uint64_t string_ids() { return *reinterpret_cast<uint64_t *>(Real() + OFFSET(DexFile, string_ids_)) & CoreApi::GetPointMask(); }

    uint8_t* DataBegin();
    dex::TypeId GetTypeId(dex::TypeIndex idx);
    const char* GetTypeDescriptor(dex::TypeId& type_id);
    const char* StringDataByIdx(dex::StringIndex idx);
    const char* StringDataAndUtf16LengthByIdx(dex::StringIndex idx, uint32_t* utf16_length);
    dex::StringId GetStringId(dex::StringIndex idx);
    const char* GetStringDataAndUtf16Length(dex::StringId& string_id, uint32_t* utf16_length);
};

} // namespace art

#endif // ANDROID_ART_DEX_DEX_FILE_H_
