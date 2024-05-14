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

#include "android.h"
#include "base/macros.h"
#include "runtime/runtime.h"
#include "runtime/art_method.h"
#include "dex/descriptors_names.h"

struct ArtMethod_OffsetTable __ArtMethod_offset__;
struct ArtMethod_SizeTable __ArtMethod_size__;
struct PtrSizedFields_OffsetTable __PtrSizedFields_offset__;

namespace art {

void ArtMethod::Init() {
    __ArtMethod_offset__ = {
        .declaring_class_ = 0,
        .access_flags_ = 4,
        .dex_code_item_offset_ = 8,
        .dex_method_index_ = 12,
        .method_index_ = 16,
        .hotness_count_ = 18,
        .imt_index_ = 18,
        .ptr_sized_fields_ = 24,
    };

    if (CoreApi::GetPointSize() == 64) {
        __ArtMethod_size__ = {
            .THIS = 40,
        };
    } else {
        __ArtMethod_size__ = {
            .THIS = 28,
        };
    }
}

void ArtMethod::Init31() {
    __ArtMethod_offset__ = {
        .declaring_class_ = 0,
        .access_flags_ = 4,
        .dex_method_index_ = 8,
        .method_index_ = 12,
        .hotness_count_ = 14,
        .imt_index_ = 14,
        .ptr_sized_fields_ = 16,
    };

    if (CoreApi::GetPointSize() == 64) {
        __ArtMethod_size__ = {
            .THIS = 32,
        };
    } else {
        __ArtMethod_size__ = {
            .THIS = 24,
        };
    }
}

void ArtMethod::PtrSizedFields::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __PtrSizedFields_offset__ = {
            .data_ = 0,
            .entry_point_from_quick_compiled_code_ = 8,
        };
    } else {
        __PtrSizedFields_offset__ = {
            .data_ = 0,
            .entry_point_from_quick_compiled_code_ = 4,
        };
    }
}

mirror::Class& ArtMethod::GetDeclaringClass() {
    if (!declaring_class_cache.Ptr()) {
        declaring_class_cache = declaring_class();
        declaring_class_cache.Prepare(false);
    }
    return declaring_class_cache;
}

ArtMethod::PtrSizedFields& ArtMethod::GetPtrSizedFields() {
    if (!ptr_sized_fields_cache.Ptr()) {
        ptr_sized_fields_cache = ptr_sized_fields();
        ptr_sized_fields_cache.copyRef(this);
        ptr_sized_fields_cache.Prepare(false);
    }
    return ptr_sized_fields_cache;
}

mirror::DexCache& ArtMethod::GetDexCache() {
    if (LIKELY(!IsObsolete())) {
        return GetDeclaringClass().GetDexCache();
    } else {
        return obsolete_dex_caches;
    }
}

DexFile& ArtMethod::GetDexFile() {
    return GetDexCache().GetDexFile();
}

dex::TypeIndex ArtMethod::GetReturnTypeIndex() {
    DexFile& dex_file = GetDexFile();
    dex::MethodId method_id = dex_file.GetMethodId(GetDexMethodIndex());
    dex::ProtoId proto_id = dex_file.GetMethodPrototype(method_id);
    dex::TypeIndex type_idx(proto_id.return_type_idx());
    return type_idx;
}

const char* ArtMethod::GetReturnTypeDescriptor() {
    DexFile& dex_file = GetDexFile();
    dex::TypeIndex type_idx(GetReturnTypeIndex());
    dex::TypeId type_id = dex_file.GetTypeId(type_idx);
    return dex_file.GetTypeDescriptor(type_id);
}

std::string ArtMethod::PrettyReturnTypeDescriptor() {
    std::string tmp;
    AppendPrettyDescriptor(GetReturnTypeDescriptor(), &tmp, "V");
    return tmp;
}

const char* ArtMethod::GetName() {
    uint32_t dex_method_idx = GetDexMethodIndex();
    if (LIKELY(dex_method_idx != dex::kDexNoIndex)) {
        DexFile& dex_file = GetDexFile();
        dex::MethodId method_id = dex_file.GetMethodId(dex_method_idx);
        return dex_file.GetMethodName(method_id);
    }
    return GetRuntimeMethodName();
}

const char* ArtMethod::GetRuntimeMethodName() {
    Runtime& runtime = Runtime::Current();
    if (*this == runtime.GetResolutionMethod()) {
        return "<runtime internal resolution method>";
    } else if (*this == runtime.GetImtConflictMethod()) {
        return "<runtime internal imt conflict method>";
    } else if (*this == runtime.GetCalleeSaveMethod(CalleeSaveType::kSaveAllCalleeSaves)) {
        return "<runtime internal callee-save all registers method>";
    } else if (*this == runtime.GetCalleeSaveMethod(CalleeSaveType::kSaveRefsOnly)) {
        return "<runtime internal callee-save reference registers method>";
    } else if (*this == runtime.GetCalleeSaveMethod(CalleeSaveType::kSaveRefsAndArgs)) {
        return "<runtime internal callee-save reference and argument registers method>";
    } else if (*this == runtime.GetCalleeSaveMethod(CalleeSaveType::kSaveEverything)) {
        return "<runtime internal save-every-register method>";
    } else if (*this == runtime.GetCalleeSaveMethod(CalleeSaveType::kSaveEverythingForClinit)) {
        return "<runtime internal save-every-register method for clinit>";
    } else if (*this == runtime.GetCalleeSaveMethod(CalleeSaveType::kSaveEverythingForSuspendCheck)) {
        return "<runtime internal save-every-register method for suspend check>";
    } else {
        return "<unknown runtime internal method>";
    }
}

std::string ArtMethod::PrettyParameters() {
    DexFile& dex_file = GetDexFile();
    dex::MethodId method_id = dex_file.GetMethodId(GetDexMethodIndex());
    return dex_file.PrettyMethodParameters(method_id);
}

std::string ArtMethod::PrettyMethod() {
    std::string result;
    uint32_t dex_method_idx = GetDexMethodIndex();
    if (LIKELY(dex_method_idx != dex::kDexNoIndex)) {
        result.append(PrettyReturnTypeDescriptor());
        result.append(" ");
        result.append(GetDeclaringClass().PrettyDescriptor());
        result.append(".");
        result.append(GetName());
        result.append(PrettyParameters());
        return result;
    }
    return GetRuntimeMethodName();
}

} //namespace art
