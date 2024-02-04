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

#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/string.h"
#include "runtime/mirror/array.h"
#include "common/exception.h"

struct Object_OffsetTable __Object_offset__;
struct Object_SizeTable __Object_size__;

namespace art {
namespace mirror {

void Object::Init() {
    __Object_offset__ = {
        .klass_ = 0,
        .monitor_ = 4,
    };

    __Object_size__ = {
        .THIS = 8,
        .klass_ = 4,
        .monitor_ = 4,
    };
}

Class Object::GetClass() {
    if (!klass_cache.Ptr()) {
        klass_cache = klass();
        klass_cache.copyRef(this);
        klass_cache.Prepare(false);
    }
    Class clazz(klass(), klass_cache);
    return clazz;
}

bool Object::IsClass() {
    Class klass_ = GetClass();
    Class java_lang_Class = klass_.GetClass();
    return klass_ == java_lang_Class;
}

bool Object::IsObjectArray() {
    Class klass_ = GetClass();
    Class component_type_ = klass_.GetComponentType();
    return IsArrayInstance() && !component_type_.IsPrimitive();
}

bool Object::IsArrayInstance() {
    return GetClass().IsArrayClass();
}

bool Object::IsReferenceInstance() {
    return GetClass().IsTypeOfReferenceClass();
}

bool Object::IsSpecificPrimitiveArray(uint32_t type) {
    Class klass_ = GetClass();
    Class component_type_ = klass_.GetComponentType();
    return component_type_ != 0x0 && component_type_.GetPrimitiveType() == type;
}

bool Object::IsBooleanArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimBoolean);
}

bool Object::IsByteArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimByte);
}

bool Object::IsCharArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimChar);
}

bool Object::IsShortArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimShort);
}

bool Object::IsIntArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimInt);
}

bool Object::IsLongArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimLong);
}

bool Object::IsFloatArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimFloat);
}

bool Object::IsDoubleArray() {
    return IsSpecificPrimitiveArray(Primitive::kPrimDouble);
}

bool Object::IsString() {
    return GetClass().IsStringClass();
}

bool Object::IsWeakReferenceInstance() {
    return GetClass().IsWeakReferenceClass();
}

bool Object::IsSoftReferenceInstance() {
    return GetClass().IsSoftReferenceClass();
}

bool Object::IsFinalizerReferenceInstance() {
    return GetClass().IsFinalizerReferenceClass();
}

bool Object::IsPhantomReferenceInstance() {
    return GetClass().IsPhantomReferenceClass();
}

bool Object::IsClassLoader() {
    return GetClass().IsClassLoaderClass();
}

bool Object::IsDexCache() {
    return GetClass().IsDexCacheClass();
}

uint64_t Object::SizeOf() {
    if (IsArrayInstance()) {
        Array array = *this;
        return array.SizeOf();
    } else if (IsClass()) {
        Class clazz = *this;
        return clazz.SizeOf();
    } else if (IsString()) {
        String value = *this;
        return value.SizeOf();
    } else {
        return GetClass().GetObjectSize();
    }
    return 0x0;
}

bool Object::IsValid() {
    Class klass_ = GetClass();
    if (klass_.Ptr() == 0x0
            || klass_.Ptr() == 0xBADDB01D)
        return false;
    try {
        if (klass_.IsClass())
            return true;
    } catch (InvalidAddressException e) {
        // do nothing
    }
    return false;
}

} // namespcae mirror
} // namespace art
