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

#include "runtime/art_field.h"
#include "dex/descriptors_names.h"

struct ArtField_OffsetTable __ArtField_offset__;
struct ArtField_SizeTable __ArtField_size__;

namespace art {

void ArtField::Init() {
    __ArtField_offset__ = {
        .declaring_class_ = 0,
        .access_flags_ = 4,
        .field_dex_idx_ = 8,
        .offset_ = 12,
    };

    __ArtField_size__ = {
        .THIS = 16,
    };
}

bool ArtField::IsProxyField() {
    return GetDeclaringClass().IsProxyClass();
}

const char* ArtField::GetTypeDescriptor() {
    uint32_t field_index = GetDexFieldIndex();
    if (IsProxyField()) {
        return field_index == 0x0 ? "[Ljava/lang/Class;" : "[[Ljava/lang/Class;";
    }
    DexFile& dex_file = GetDexFile();
    dex::FieldId id = dex_file.GetFieldId(field_index);
    return dex_file.GetFieldTypeDescriptor(id);
}

std::string ArtField::PrettyTypeDescriptor() {
    std::string tmp;
    AppendPrettyDescriptor(GetTypeDescriptor(), &tmp, "byte");
    return tmp;
}

mirror::Class ArtField::GetDeclaringClass() {
    mirror::Class declaring_class_(declaring_class(), this);
    return declaring_class_;
}

uint32_t ArtField::GetDexFieldIndex() {
    return field_dex_idx();
}

mirror::DexCache ArtField::GetDexCache() {
    return GetDeclaringClass().GetDexCache();
}

DexFile& ArtField::GetDexFile() {
    if (!dex_file_cache.Ptr()) {
        dex_file_cache = GetDexCache().GetDexFile();
    }
    return dex_file_cache;
}

uint32_t ArtField::GetOffset() {
    return offset();
}

const char* ArtField::GetName() {
    int32_t field_index = GetDexFieldIndex();
    if (IsProxyField()) {
        return field_index == 0 ? "interfaces" : "throws";
    }
    DexFile& dex_file = GetDexFile();
    dex::FieldId id = dex_file.GetFieldId(field_index);
    return dex_file.GetFieldName(id);
}

uint32_t ArtField::GetAccessFlags() {
    return access_flags();
}

int8_t ArtField::GetByte(mirror::Object& obj) {
    return *reinterpret_cast<int8_t *>(obj.Real() + offset());
}

uint8_t ArtField::GetBoolean(mirror::Object& obj) {
    return *reinterpret_cast<uint8_t *>(obj.Real() + offset());
}

uint16_t ArtField::GetChar(mirror::Object& obj) {
    return *reinterpret_cast<uint16_t *>(obj.Real() + offset());
}

int16_t ArtField::GetShort(mirror::Object& obj) {
    return *reinterpret_cast<int16_t *>(obj.Real() + offset());
}

int32_t ArtField::GetInt(mirror::Object& obj) {
    return *reinterpret_cast<int32_t *>(obj.Real() + offset());
}

int64_t ArtField::GetLong(mirror::Object& obj) {
    return *reinterpret_cast<int64_t *>(obj.Real() + offset());
}

float ArtField::GetFloat(mirror::Object& obj) {
    return *reinterpret_cast<float *>(obj.Real() + offset());
}

double ArtField::GetDouble(mirror::Object& obj) {
    return *reinterpret_cast<double *>(obj.Real() + offset());
}

uint32_t ArtField::Get32(mirror::Object& obj) {
    return *reinterpret_cast<uint32_t *>(obj.Real() + offset());
}

uint64_t ArtField::Get64(mirror::Object& obj) {
    return *reinterpret_cast<uint64_t *>(obj.Real() + offset());
}

uint32_t ArtField::GetObj(mirror::Object& obj) {
    return *reinterpret_cast<uint32_t *>(obj.Real() + offset());
}

} // namespace art
