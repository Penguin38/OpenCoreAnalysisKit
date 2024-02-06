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

#ifndef ANDROID_ART_RUNTIME_MIRROR_CLASS_H_
#define ANDROID_ART_RUNTIME_MIRROR_CLASS_H_

#include "api/memory_ref.h"
#include "runtime/art_field.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/string.h"
#include "runtime/mirror/dex_cache.h"
#include "runtime/class_status.h"
#include "dex/primitive.h"
#include "dex/dex_file.h"
#include <string>

struct Class_OffsetTable {
    uint32_t class_loader_;
    uint32_t component_type_;
    uint32_t dex_cache_;
    uint32_t ext_data_;
    uint32_t iftable_;
    uint32_t name_;
    uint32_t super_class_;
    uint32_t vtable_;
    uint32_t ifields_;
    uint32_t methods_;
    uint32_t sfields_;
    uint32_t access_flags_;
    uint32_t class_flags_;
    uint32_t class_size_;
    uint32_t clinit_thread_id_;
    uint32_t dex_class_def_idx_;
    uint32_t dex_type_idx_;
    uint32_t num_reference_instance_fields_;
    uint32_t num_reference_static_fields_;
    uint32_t object_size_;
    uint32_t object_size_alloc_fast_path_;
    uint32_t primitive_type_;
    uint32_t reference_instance_offsets_;
    uint32_t status_;
    uint32_t copied_methods_offset_;
    uint32_t virtual_methods_offset_;
};

struct Class_SizeTable {
    uint32_t THIS;
};

extern struct Class_OffsetTable __Class_offset__;
extern struct Class_SizeTable __Class_size__;

namespace art {
namespace mirror {

constexpr static uint32_t kPrimitiveTypeSizeShiftShift = 16;
constexpr static uint32_t kPrimitiveTypeMask = (1u << kPrimitiveTypeSizeShiftShift) - 1;

class Class : public Object {
public:
    Class(uint32_t v) : Object(v) {}
    Class(const Object& ref) : Object(ref) {}
    Class(const api::MemoryRef& ref) : Object(ref) {}
    Class(uint32_t v, Object* ref) : Object(v, ref) {}
    Class(uint32_t v, api::MemoryRef& ref) : Object(v, ref) {}
    Class(uint32_t v, api::MemoryRef* ref) : Object(v, ref) {}

    inline bool operator==(Object& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Object& ref) { return Ptr() != ref.Ptr(); }
    inline bool operator==(uint32_t v) { return Ptr() == v; }
    inline bool operator!=(uint32_t v) { return Ptr() != v; }
    inline bool operator<(const Class& ref) const { return Ptr() < ref.Ptr(); }
    inline bool operator>(const Class& ref) const { return Ptr() > ref.Ptr(); }

    static void Init();
    inline uint32_t class_loader() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, class_loader_)); }
    inline uint32_t component_type() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, component_type_)); }
    inline uint32_t dex_cache() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, dex_cache_)); }
    inline uint32_t ext_data() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, ext_data_)); }
    inline uint32_t iftable() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, iftable_)); }
    inline uint32_t name() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, name_)); }
    inline uint32_t super_class() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, super_class_)); }
    inline uint32_t vtable() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, vtable_)); }
    inline uint64_t ifields() { return *reinterpret_cast<uint64_t*>(Real() + OFFSET(Class, ifields_)); }
    inline uint64_t methods() { return *reinterpret_cast<uint64_t*>(Real() + OFFSET(Class, methods_)); }
    inline uint64_t sfields() { return *reinterpret_cast<uint64_t*>(Real() + OFFSET(Class, sfields_)); }
    inline uint32_t access_flags() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, access_flags_)); }
    inline uint32_t class_flags() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, class_flags_)); }
    inline uint32_t class_size() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, class_size_)); }
    inline uint32_t clinit_thread_id() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, clinit_thread_id_)); }
    inline uint32_t dex_class_def_idx() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, dex_class_def_idx_)); }
    inline uint32_t dex_type_idx() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, dex_type_idx_)); }
    inline uint32_t num_reference_instance_fields() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, num_reference_instance_fields_)); }
    inline uint32_t num_reference_static_fields() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, num_reference_static_fields_)); }
    inline uint32_t object_size() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, object_size_)); }
    inline uint32_t object_size_alloc_fast_path() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, object_size_alloc_fast_path_)); }
    inline uint32_t primitive_type() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, primitive_type_)); }
    inline uint32_t reference_instance_offsets() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, reference_instance_offsets_)); }
    inline uint32_t status() { return *reinterpret_cast<uint32_t*>(Real() + OFFSET(Class, status_)); }
    inline uint16_t copied_methods_offset() { return *reinterpret_cast<uint16_t*>(Real() + OFFSET(Class, copied_methods_offset_)); }
    inline uint16_t virtual_methods_offset() { return *reinterpret_cast<uint16_t*>(Real() + OFFSET(Class, virtual_methods_offset_)); }

    bool IsArrayClass();
    bool IsStringClass();
    bool IsDexCacheClass();
    bool IsClassLoaderClass();
    bool IsClassClass();
    bool IsObjectClass();
    bool IsPrimitive();
    bool IsPrimitiveBoolean();
    bool IsPrimitiveByte();
    bool IsPrimitiveChar();
    bool IsPrimitiveShort();
    bool IsPrimitiveInt();
    bool IsPrimitiveLong();
    bool IsPrimitiveFloat();
    bool IsPrimitiveDouble();
    bool IsPrimitiveVoid();
    bool IsTypeOfReferenceClass();
    bool IsWeakReferenceClass();
    bool IsSoftReferenceClass();
    bool IsFinalizerReferenceClass();
    bool IsPhantomReferenceClass();
    bool IsRetired();
    bool IsErroneousUnresolved();
    bool IsErroneousResolved();
    bool IsErroneous();
    bool IsIdxLoaded();
    bool IsResolved();
    bool ShouldVerifyAtRuntime();
    bool IsVerifiedNeedsAccessChecks();
    bool IsVerified();
    bool IsInitializing();
    bool IsVisiblyInitialized();
    bool IsEnum();
    bool IsInterface();
    bool IsPublic();
    bool IsFinal();
    bool IsFinalizable();
    bool ShouldSkipHiddenApiChecks();
    bool IsAbstract();
    bool IsAnnotation();
    bool IsSynthetic();
    bool WasVerificationAttempted();
    bool IsObsoleteObject();
    bool IsProxyClass();

    Primitive::Type GetPrimitiveType();
    Class GetComponentType();
    uint32_t GetClassFlags();
    Class GetSuperClass();
    uint64_t GetComponentSizeShift();
    uint64_t GetPrimitiveTypeSizeShift();
    uint64_t GetObjectSize();
    ClassStatus GetStatus();
    uint32_t GetAccessFlags();
    uint32_t SizeOf();
    uint32_t GetClassSize();
    std::string PrettyDescriptor();
    const char *GetDescriptor(std::string* storage);
    String GetName();
    DexFile& GetDexFile();
    DexCache& GetDexCache();
    dex::TypeIndex GetDexTypeIndex();
    uint32_t NumInstanceFields();
    uint64_t GetIFields();
    uint32_t NumStaticFields();
    uint64_t GetSFields();
    Class GetClassLoader();

private:
    // quick memoryref cache
    DexCache dex_cache_cache = 0x0;
};

} // namespace mirror
} // namespace art

#endif  // ANDROID_ART_RUNTIME_MIRROR_CLASS_H_
