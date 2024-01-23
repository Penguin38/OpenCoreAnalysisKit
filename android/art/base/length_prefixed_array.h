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

#ifndef ANDROID_ART_BASE_LENGTH_PREFIXED_ARRAY_H_
#define ANDROID_ART_BASE_LENGTH_PREFIXED_ARRAY_H_

#include "api/memory_ref.h"

struct LengthPrefixedArray_OffsetTable {
    uint32_t size_;
    uint32_t data_;
};

struct LengthPrefixedArray_SizeTable {
    uint32_t THIS;
};

extern struct LengthPrefixedArray_OffsetTable __LengthPrefixedArray_offset__;
extern struct LengthPrefixedArray_SizeTable __LengthPrefixedArray_size__;

namespace art {

class LengthPrefixedArray : public api::MemoryRef {
public:
    LengthPrefixedArray(uint64_t v) : api::MemoryRef(v) {}
    LengthPrefixedArray(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    LengthPrefixedArray(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> LengthPrefixedArray(U *v) : api::MemoryRef(v) {}
    template<typename U> LengthPrefixedArray(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(LengthPrefixedArray& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(LengthPrefixedArray& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t size() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(LengthPrefixedArray, size_)); }
    inline uint64_t data() { return Ptr() + OFFSET(LengthPrefixedArray, data_); }
};

} //namespace art

#endif  // ANDROID_ART_BASE_LENGTH_PREFIXED_ARRAY_H_
