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

#ifndef ANDROID_ART_RUNTIME_ART_FIELD_H_
#define ANDROID_ART_RUNTIME_ART_FIELD_H_

#include "logger/log.h"
#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/dex_cache.h"
#include "dex/dex_file.h"

struct ArtField_OffsetTable {
    uint32_t declaring_class_;
    uint32_t access_flags_;
    uint32_t field_dex_idx_;
    uint32_t offset_;
};

struct ArtField_SizeTable {
    uint32_t THIS;
};

extern struct ArtField_OffsetTable __ArtField_offset__;
extern struct ArtField_SizeTable __ArtField_size__;

namespace art {

class ArtField : public api::MemoryRef {
public:
    ArtField(uint64_t v) : api::MemoryRef(v) {}
    ArtField(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ArtField(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ArtField(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(ArtField& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(ArtField& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t declaring_class() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(ArtField, declaring_class_)); }
    inline uint32_t access_flags() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(ArtField, access_flags_)); }
    inline uint32_t field_dex_idx() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(ArtField, field_dex_idx_)); }
    inline uint32_t offset() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(ArtField, offset_)); }

    bool IsProxyField();
    const char* GetTypeDescriptor();
    std::string PrettyTypeDescriptor();
    mirror::Class GetDeclaringClass();
    uint32_t GetDexFieldIndex();
    mirror::DexCache GetDexCache();
    DexFile& GetDexFile();
    const char* GetName();
    inline uint32_t GetOffset() { return offset(); }
    inline uint32_t GetAccessFlags() { return access_flags(); }

    inline int8_t GetByte(mirror::Object& obj) { return *reinterpret_cast<uint8_t *>(obj.Real() + offset()); }
    inline uint8_t GetBoolean(mirror::Object& obj) { return *reinterpret_cast<uint8_t *>(obj.Real() + offset()); }
    inline uint16_t GetChar(mirror::Object& obj) { return *reinterpret_cast<uint16_t *>(obj.Real() + offset()); }
    inline int16_t GetShort(mirror::Object& obj) { return *reinterpret_cast<uint16_t *>(obj.Real() + offset()); }
    inline int32_t GetInt(mirror::Object& obj) { return *reinterpret_cast<uint32_t *>(obj.Real() + offset()); }
    inline int64_t GetLong(mirror::Object& obj) { return *reinterpret_cast<uint64_t *>(obj.Real() + offset()); }
    inline float GetFloat(mirror::Object& obj) { return *reinterpret_cast<uint32_t *>(obj.Real() + offset()); }
    inline double GetDouble(mirror::Object& obj) { return *reinterpret_cast<uint64_t *>(obj.Real() + offset()); }
    inline uint32_t Get32(mirror::Object& obj) { return *reinterpret_cast<uint32_t *>(obj.Real() + offset()); }
    inline uint64_t Get64(mirror::Object& obj) { return *reinterpret_cast<uint64_t *>(obj.Real() + offset()); }
    inline uint32_t GetObj(mirror::Object& obj) { return *reinterpret_cast<uint32_t *>(obj.Real() + offset()); }
    inline uint32_t GetObject(mirror::Object& obj) { return *reinterpret_cast<uint32_t *>(obj.Real() + offset()); }

private:
    // quick memoryref cache
    DexFile dex_file_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_ART_FIELD_H_
