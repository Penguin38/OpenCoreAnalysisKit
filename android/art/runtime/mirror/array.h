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

#ifndef ANDROID_ART_RUNTIME_MIRROR_ARRAY_H_
#define ANDROID_ART_RUNTIME_MIRROR_ARRAY_H_

#include "api/memory_ref.h"
#include "runtime/mirror/object.h"

struct Array_OffsetTable {
    uint32_t length_;
    uint32_t first_element_;
};

struct Array_SizeTable {
    uint32_t THIS;
    uint32_t length_;
    uint32_t first_element_;
};

extern struct Array_OffsetTable __Array_offset__;
extern struct Array_SizeTable __Array_size__;

namespace art {
namespace mirror {

class Array : public Object {
public:
    Array(uint32_t v) : Object(v) {}
    Array(const Object& ref) : Object(ref) {}
    Array(uint32_t v, Object* ref) : Object(v, ref) {}

    inline bool operator==(Object& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Object& ref) { return Ptr() != ref.Ptr(); }
    inline bool operator==(uint32_t v) { return Ptr() == v; }
    inline bool operator!=(uint32_t v) { return Ptr() != v; }

    static void Init();
    inline int32_t length() { return *reinterpret_cast<int32_t*>(Real() + OFFSET(Array, length_)); }
    inline uint32_t first_element() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Array, first_element_)); }

    int32_t GetLength();
    uint64_t GetRawData(size_t component_size, int32_t index);
    uint64_t SizeOf();
};

} // namespace mirror
} // namespace art

#endif  // ANDROID_ART_RUNTIME_MIRROR_ARRAY_H_
