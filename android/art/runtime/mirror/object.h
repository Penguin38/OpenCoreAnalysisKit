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

#ifndef ANDROID_ART_RUNTIME_MIRROR_OBJECT_H_
#define ANDROID_ART_RUNTIME_MIRROR_OBJECT_H_

#include "api/memory_ref.h"

struct Object_OffsetTable {
    uint32_t klass_;
    uint32_t monitor_;
};

struct Object_SizeTable {
    uint32_t THIS;
    uint32_t klass_;
    uint32_t monitor_;
};

extern struct Object_OffsetTable __Object_offset__;
extern struct Object_SizeTable __Object_size__;

namespace art {
namespace mirror {

class Class;

class Object : public api::MemoryRef {
public:
    Object(uint32_t v) : api::MemoryRef(v) {}
    Object(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Object(uint32_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Object(uint32_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> Object(U *v) : api::MemoryRef(v) {}
    template<typename U> Object(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(Object& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Object& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t klass() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Object, klass_)); }
    inline uint32_t monitor() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Object, monitor_)); }

    Class GetClass();
    bool IsClass();
    bool IsObjectArray();
    bool IsArrayInstance();
    bool IsReferenceInstance();
    bool IsSpecificPrimitiveArray(uint32_t type);
    bool IsBooleanArray();
    bool IsByteArray();
    bool IsCharArray();
    bool IsShortArray();
    bool IsIntArray();
    bool IsLongArray();
    bool IsFloatArray();
    bool IsDoubleArray();
    bool IsString();
    bool IsWeakReferenceInstance();
    bool IsSoftReferenceInstance();
    bool IsFinalizerReferenceInstance();
    bool IsPhantomReferenceInstance();
    bool IsClassLoader();
    bool IsDexCache();
    uint64_t SizeOf();
};

} // namespace mirror
} // namespace art

#endif  // ANDROID_ART_RUNTIME_MIRROR_OBJECT_H_
