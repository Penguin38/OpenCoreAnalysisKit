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

#ifndef ANDROID_ART_RUNTIME_MIRROR_STRING_H_
#define ANDROID_ART_RUNTIME_MIRROR_STRING_H_

#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include <string>

struct String_OffsetTable {
    uint32_t count_;
    uint32_t hash_code_;
    uint32_t value_;
    uint32_t value_compressed_;
};

struct String_SizeTable {
    uint32_t THIS;
};

extern struct String_OffsetTable __String_offset__;
extern struct String_SizeTable __String_size__;

enum class StringCompressionFlag : uint32_t {
    kCompressed = 0u,
    kUncompressed = 1u
};

namespace art {
namespace mirror {

class String : public Object {
public:
    String(uint32_t v) : Object(v) {}
    String(const Object& ref) : Object(ref) {}
    String(uint32_t v, Object* ref) : Object(v, ref) {}

    inline bool operator==(Object& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Object& ref) { return Ptr() != ref.Ptr(); }
    inline bool operator==(uint32_t v) { return Ptr() == v; }
    inline bool operator!=(uint32_t v) { return Ptr() != v; }

    static void Init();
    inline int32_t count() { return *reinterpret_cast<int32_t *>(Real() + OFFSET(String, count_)); }
    inline uint32_t hash_code() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(String, hash_code_)); }
    inline uint16_t* value() { return reinterpret_cast<uint16_t *>(Real() + OFFSET(String, value_)); }
    inline uint8_t* value_compressed() { return reinterpret_cast<uint8_t *>(Real() + OFFSET(String, value_compressed_)); }

    int32_t GetCount();
    int32_t GetLength();
    int32_t GetLengthFromCount(int32_t count);
    bool IsCompressed();
    StringCompressionFlag GetCompressionFlagFromCount(int32_t count);
    uint8_t* GetValueCompressed();
    uint16_t* GetValue();
    std::string ToModifiedUtf8();
    int32_t GetUtfLength();
    uint64_t SizeOf();
};

} // namespace mirror
} // namespace art

#endif  // ANDROID_ART_RUNTIME_MIRROR_STRING_H_
