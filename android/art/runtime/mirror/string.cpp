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

#include "android.h"
#include "runtime/mirror/string.h"
#include "runtime/runtime_globals.h"
#include "dex/utf.h"
#include "common/bit.h"

struct String_OffsetTable __String_offset__;
struct String_SizeTable __String_size__;

namespace art {
namespace mirror {

void String::Init() {
    __String_offset__ = {
        .count_ = 8,
        .hash_code_ = 12,
        .value_ = 16,
        .value_compressed_ = 16,
    };

    __String_size__ = {
        .THIS = 16,
    };
}

int32_t String::GetCount() {
    return count();
}

int32_t String::GetLength() {
    if (Android::Sdk() >= Android::O)
        return GetLengthFromCount(GetCount());
    else
        return GetCount();
}

int32_t String::GetLengthFromCount(int32_t count) {
    return static_cast<int32_t>(static_cast<uint32_t>(count) >> 1);
}

bool String::IsCompressed() {
    if (Android::Sdk() >= Android::O)
        return GetCompressionFlagFromCount(GetCount()) == StringCompressionFlag::kCompressed;
    else
        return false;
}

StringCompressionFlag String::GetCompressionFlagFromCount(int32_t count) {
    return static_cast<StringCompressionFlag>(static_cast<uint32_t>(count) & 1u);
}

uint8_t* String::GetValueCompressed() {
    return value_compressed();
}

uint16_t* String::GetValue() {
    return value();
}

std::string String::ToModifiedUtf8() {
    if (IsCompressed()) {
        return std::string(reinterpret_cast<const char*>(GetValueCompressed()), GetLength());
    } else {
        uint64_t byte_count = GetUtfLength();
        std::string result(byte_count, static_cast<char>(0));
        ConvertUtf16ToModifiedUtf8(&result[0], byte_count, GetValue(), GetLength());
        return result;
    }
}

int32_t String::GetUtfLength() {
    if (IsCompressed()) {
        return GetLength();
    } else {
        return CountUtf8Bytes(GetValue(), GetLength());
    }
}

uint64_t String::SizeOf() {
    uint64_t size = SIZEOF(String);
    if (IsCompressed()) {
        size += sizeof(uint8_t) * GetLength();
    } else {
        size += sizeof(uint16_t) * GetLength();
    }
    return RoundUp(size, kObjectAlignment);
}

} // namespace mirror
} // namespace art
