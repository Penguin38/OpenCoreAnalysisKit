/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") = 
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
#include "android.h"
#include "common/bit.h"
#include "api/core.h"
#include "base/macros.h"
#include "runtime/mirror/class.h"
#include "dex/descriptors_names.h"
#include "base/length_prefixed_array.h"

struct Class_OffsetTable __Class_offset__;
struct Class_SizeTable __Class_size__;

namespace art {
namespace mirror {

void Class::Init() {
    Android::RegisterSdkListener(Android::M, art::mirror::Class::Init23);
    Android::RegisterSdkListener(Android::N, art::mirror::Class::Init24);
    Android::RegisterSdkListener(Android::O, art::mirror::Class::Init26);
}

void Class::Init23() {
    __Class_offset__ = {
        .class_loader_ = 8,
        .component_type_ = 12,
        .dex_cache_ = 16,
        .iftable_ = 24,
        .name_ = 28,
        .super_class_ = 32,
        .vtable_ = 40,
        .ifields_ = 56,
        .sfields_ = 64,
        .access_flags_ = 44,
        .class_size_ = 80,
        .clinit_thread_id_ = 84,
        .dex_class_def_idx_ = 88,
        .dex_type_idx_ = 92,
        .num_reference_instance_fields_ = 104,
        .num_reference_static_fields_ = 108,
        .object_size_ = 120,
        .primitive_type_ = 124,
        .reference_instance_offsets_ = 128,
        .status_ = 132,
        .dex_cache_strings_ = 20,
        .direct_methods_ = 48,
        .virtual_methods_ = 72,
        .num_direct_methods_ = 96,
        .num_instance_fields_ = 100,
        .num_static_fields_ = 112,
        .num_virtual_methods_ = 116,
    };

    __Class_size__ = {
        .THIS = 136,
    };

}

void Class::Init24() {
    __Class_offset__ = {
        .class_loader_ = 12,
        .component_type_ = 16,
        .dex_cache_ = 20,
        .iftable_ = 24,
        .name_ = 28,
        .super_class_ = 32,
        .vtable_ = 40,
        .ifields_ = 56,
        .methods_ = 64,
        .sfields_ = 72,
        .access_flags_ = 44,
        .class_flags_ = 80,
        .class_size_ = 84,
        .clinit_thread_id_ = 88,
        .dex_class_def_idx_ = 92,
        .dex_type_idx_ = 96,
        .num_reference_instance_fields_ = 100,
        .num_reference_static_fields_ = 104,
        .object_size_ = 108,
        .primitive_type_ = 112,
        .reference_instance_offsets_ = 118,
        .status_ = 122,
        .copied_methods_offset_ = 126,
        .virtual_methods_offset_ = 128,
        .dex_cache_strings_ = 48,
    };

    __Class_size__ = {
        .THIS = 130,
    };
}

void Class::Init26() {
    __Class_offset__ = {
        .class_loader_ = 8,
        .component_type_ = 12,
        .dex_cache_ = 16,
        .ext_data_ = 20,
        .iftable_ = 24,
        .name_ = 28,
        .super_class_ = 32,
        .vtable_ = 36,
        .ifields_ = 40,
        .methods_ = 48,
        .sfields_ = 56,
        .access_flags_ = 64,
        .class_flags_ = 68,
        .class_size_ = 72,
        .clinit_thread_id_ = 76,
        .dex_class_def_idx_ = 80,
        .dex_type_idx_ = 84,
        .num_reference_instance_fields_ = 88,
        .num_reference_static_fields_ = 92,
        .object_size_ = 96,
        .object_size_alloc_fast_path_ = 100,
        .primitive_type_ = 104,
        .reference_instance_offsets_ = 108,
        .status_ = 112,
        .copied_methods_offset_ = 116,
        .virtual_methods_offset_ = 118,
    };

    __Class_size__ = {
        .THIS = 120,
    };
}

bool Class::IsArrayClass() {
    return GetComponentType() != 0x0;
}

bool Class::IsStringClass() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return (GetClassFlags() & kClassFlagString) != 0x0;
    else
        return (GetAccessFlags() & kAccClassIsStringClass) != 0x0;
}

bool Class::IsObjectClass() {
    return !IsPrimitive() && GetSuperClass() == 0x0;
}

bool Class::IsPrimitive() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimNot;
}

bool Class::IsPrimitiveBoolean() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimBoolean;
}

bool Class::IsPrimitiveByte() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimByte;
}

bool Class::IsPrimitiveChar() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimChar;
}

bool Class::IsPrimitiveShort() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimShort;
}

bool Class::IsPrimitiveInt() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimInt;
}

bool Class::IsPrimitiveLong() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimLong;
}

bool Class::IsPrimitiveFloat() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimFloat;
}

bool Class::IsPrimitiveDouble() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimDouble;
}

bool Class::IsPrimitiveVoid() {
    return (primitive_type() & kPrimitiveTypeMask) != Primitive::kPrimVoid;
}

bool Class::IsTypeOfReferenceClass() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return (GetClassFlags() & kClassFlagReference) != 0x0;
    else
        return (GetAccessFlags() & kAccClassIsReference) != 0x0;
}

bool Class::IsWeakReferenceClass() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return GetClassFlags() == kClassFlagWeakReference;
    else
        return (GetAccessFlags() & kAccClassIsWeakReference) != 0x0;
}

bool Class::IsSoftReferenceClass() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return GetClassFlags() == kClassFlagSoftReference;
    else
        return (GetAccessFlags() & kAccReferenceFlagsMask) == kAccClassIsReference;
}

bool Class::IsFinalizerReferenceClass() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return GetClassFlags() == kClassFlagFinalizerReference;
    else
        return (GetAccessFlags() & kAccClassIsFinalizerReference) != 0x0;
}

bool Class::IsPhantomReferenceClass() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return GetClassFlags() == kClassFlagPhantomReference;
    else
        return (GetAccessFlags() & kAccClassIsPhantomReference) != 0x0;
}

bool Class::IsRetired() {
    return GetStatus() == ClassStatus::kRetired;
}

bool Class::IsErroneousUnresolved() {
    return GetStatus() == ClassStatus::kErrorUnresolved;
}

bool Class::IsErroneousResolved() {
    return GetStatus() == ClassStatus::kErrorResolved;
}

bool Class::IsErroneous() {
    ClassStatus status = GetStatus();
    return status == ClassStatus::kErrorUnresolved || status == ClassStatus::kErrorResolved;
}

bool Class::IsIdxLoaded() {
    return GetStatus() >= ClassStatus::kIdx;
}

bool Class::ShouldVerifyAtRuntime() {
    return GetStatus() == ClassStatus::kRetryVerificationAtRuntime;
}

bool Class::IsVerifiedNeedsAccessChecks() {
    return GetStatus() >= ClassStatus::kVerifiedNeedsAccessChecks;
}

bool Class::IsVerified() {
    return GetStatus() >= ClassStatus::kVerified;
}

bool Class::IsInitializing() {
    return GetStatus() >= ClassStatus::kInitializing;
}

bool Class::IsVisiblyInitialized() {
    return GetStatus() == ClassStatus::kVisiblyInitialized;
}

bool Class::IsEnum() {
    return (GetAccessFlags() & kAccEnum) != 0;
}

bool Class::IsInterface() {
    return (GetAccessFlags() & kAccInterface) != 0;
}

bool Class::IsPublic() {
    return (GetAccessFlags() & kAccPublic) != 0;
}

bool Class::IsFinal() {
    return (GetAccessFlags() & kAccFinal) != 0;
}

bool Class::IsFinalizable() {
    return (GetAccessFlags() & kAccClassIsFinalizable) != 0;
}

bool Class::ShouldSkipHiddenApiChecks() {
    return (GetAccessFlags() & kAccSkipHiddenapiChecks) != 0;
}

bool Class::IsAbstract() {
    return (GetAccessFlags() & kAccAbstract) != 0;
}

bool Class::IsAnnotation() {
    return (GetAccessFlags() & kAccAnnotation) != 0;
}

bool Class::IsSynthetic() {
    return (GetAccessFlags() & kAccSynthetic) != 0;
}

bool Class::WasVerificationAttempted() {
    return (GetAccessFlags() & kAccVerificationAttempted) != 0;
}

bool Class::IsObsoleteObject() {
    return (GetAccessFlags() & kAccObsoleteObject) != 0;
}

Primitive::Type Class::GetPrimitiveType() {
    int32_t v32 = primitive_type();
    Primitive::Type type = static_cast<Primitive::Type>(v32 & kPrimitiveTypeMask);
    return type;
}

Class Class::GetComponentType() {
    Class component_type_(component_type(), this);
    return component_type_;
}

Class Class::GetSuperClass() {
    Class super_class_(super_class(), this);
    return super_class_;
}

uint64_t Class::GetComponentSizeShift() {
    return GetComponentType().GetPrimitiveTypeSizeShift();
}

uint64_t Class::GetPrimitiveTypeSizeShift() {
    int32_t v32 = primitive_type();
    uint64_t size_shift = v32 >> kPrimitiveTypeSizeShiftShift;
    return size_shift;
}

std::string Class::PrettyDescriptor() {
    std::string temp;
    std::string result;
    AppendPrettyDescriptor(GetDescriptor(&temp), &result);
    return result;
}

const char* Class::GetDescriptor(std::string* storage) {
    uint64_t dim = 0u;
    Class* klass = this;
    Class tmp = 0x0;

    while (klass->Ptr() && klass->IsArrayClass()) {
        ++dim;
        tmp = klass->GetComponentType();
        klass = &tmp;
    }

    if (klass->IsProxyClass()) {
        *storage = DotToDescriptor(klass->GetName().ToModifiedUtf8().c_str());
    } else {
        const char* descriptor;
        if (klass->IsPrimitive()) {
            descriptor = Primitive::Descriptor(klass->GetPrimitiveType());
        } else {
            DexFile& dex_file = klass->GetDexFile();
            dex::TypeId type_id = dex_file.GetTypeId(klass->GetDexTypeIndex());
            descriptor = dex_file.GetTypeDescriptor(type_id);
        }
        if (dim == 0x0) {
            return descriptor;
        }
        *storage = descriptor;
    }
    storage->insert(0u, dim, '[');
    return storage->c_str();
}

String Class::GetName() {
    String name_(name(), this);
    return name_;
}

dex::TypeIndex Class::GetDexTypeIndex() {
    return dex::TypeIndex(static_cast<uint16_t>(dex_type_idx()));
}

uint32_t Class::NumInstanceFields() {
    if (LIKELY(Android::Sdk() >= Android::N)) {
        LengthPrefixedArray arr(ifields(), this);
        return arr.Ptr() ? arr.size() : 0u;
    } else
        return num_instance_fields();
}

uint64_t Class::GetIFields() {
    if (LIKELY(Android::Sdk() >= Android::N)) {
        LengthPrefixedArray arr(ifields(), this);
        return arr.Ptr() ? arr.data() : 0u;
    } else
        return ifields();
}

uint32_t Class::NumStaticFields() {
    if (LIKELY(Android::Sdk() >= Android::N)) {
        LengthPrefixedArray arr(sfields(), this);
        return arr.Ptr() ? arr.size() : 0u;
    } else
        return num_static_fields();
}

uint64_t Class::GetSFields() {
    if (LIKELY(Android::Sdk() >= Android::N)) {
        LengthPrefixedArray arr(sfields(), this);
        return arr.Ptr() ? arr.data() : 0u;
    } else
        return sfields();
}

Class Class::GetClassLoader() {
    Class class_loader_(class_loader(), this);
    return class_loader_;
}

uint32_t Class::NumMethods() {
    if (LIKELY(Android::Sdk() >= Android::N)) {
        LengthPrefixedArray arr(methods(), this);
        return arr.Ptr() ? arr.size() : 0u;
    } else
        return NumDirectMethods() + NumVirtualMethods();
}

uint64_t Class::GetMethods() {
    LengthPrefixedArray arr(methods(), this);
    return arr.Ptr() ? RoundUp(arr.data(), CoreApi::GetPointSize()) : 0u;
}

uint32_t Class::NumDirectMethods() {
    if (LIKELY(Android::Sdk() >= Android::N))
        return GetVirtualMethodsStartOffset();
    else
        return num_direct_methods();
}

uint64_t Class::GetDirectMethods() {
    return direct_methods();
}

uint32_t Class::NumVirtualMethods() {
    return num_virtual_methods();
}

uint64_t Class::GetVirtualMethods() {
    return virtual_methods();
}

} // namespcae mirror
} // namespace art
