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

#include "logger/log.h"
#include "android.h"
#include "base/macros.h"
#include "base/enums.h"
#include "runtime/runtime.h"
#include "runtime/art_method.h"
#include "runtime/entrypoints/runtime_asm_entrypoints.h"
#include "runtime/oat/oat_file.h"
#include "dex/descriptors_names.h"
#include "dex/standard_dex_file.h"
#include "dex/compact_dex_file.h"

struct ArtMethod_OffsetTable __ArtMethod_offset__;
struct ArtMethod_SizeTable __ArtMethod_size__;
struct PtrSizedFields_OffsetTable __PtrSizedFields_offset__;

namespace art {

void ArtMethod::Init26() {
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

    if (CoreApi::Bits() == 64) {
        __ArtMethod_size__ = {
            .THIS = 48,
        };
    } else {
        __ArtMethod_offset__.ptr_sized_fields_ = 20;
        __ArtMethod_size__ = {
            .THIS = 32,
        };
    }
}

void ArtMethod::Init28() {
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

    if (CoreApi::Bits() == 64) {
        __ArtMethod_size__ = {
            .THIS = 40,
        };
    } else {
        __ArtMethod_offset__.ptr_sized_fields_ = 20;
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

    if (CoreApi::Bits() == 64) {
        __ArtMethod_size__ = {
            .THIS = 32,
        };
    } else {
        __ArtMethod_size__ = {
            .THIS = 24,
        };
    }
}

void ArtMethod::PtrSizedFields::Init26() {
    if (CoreApi::Bits() == 64) {
        __PtrSizedFields_offset__ = {
            .data_ = 8,
            .entry_point_from_quick_compiled_code_ = 16,
        };
    } else {
        __PtrSizedFields_offset__ = {
            .data_ = 4,
            .entry_point_from_quick_compiled_code_ = 8,
        };
    }
}

void ArtMethod::PtrSizedFields::Init28() {
    if (CoreApi::Bits() == 64) {
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

std::string ArtMethod::PrettyMethodOnlyNP() {
    std::string result;
    uint32_t dex_method_idx = GetDexMethodIndex();
    if (LIKELY(dex_method_idx != dex::kDexNoIndex)) {
        result.append(GetDeclaringClass().PrettyDescriptor());
        result.append(".");
        result.append(GetName());
        result.append(PrettyParameters());
        return result;
    }
    return GetRuntimeMethodName();
}

std::string ArtMethod::PrettyMethodSimple() {
    std::string result;
    uint32_t dex_method_idx = GetDexMethodIndex();
    if (LIKELY(dex_method_idx != dex::kDexNoIndex)) {
        result.append(GetDeclaringClass().PrettyDescriptor());
        result.append(".");
        result.append(GetName());
        return result;
    }
    return GetRuntimeMethodName();
}

bool ArtMethod::HasCodeItem() {
    uint32_t access_flags = GetAccessFlags();
    bool status_flag = !IsNative(access_flags) &&
                       !IsAbstract(access_flags) &&
                       !IsRuntimeMethod() &&
                       !IsProxyMethod();
    if (Android::Sdk() > Android::UPSIDE_DOWN_CAKE)
        status_flag &= !IsDefaultConflicting(access_flags);
    return status_flag;
}

dex::CodeItem ArtMethod::GetCodeItem() {
    dex::CodeItem item = 0x0;
    if (!HasCodeItem())
        return item;

    DexFile& dex_file = GetDexFile();
    if (Android::Sdk() < Android::S) {
        item = dex_file.DataBegin() + dex_code_item_offset();
    } else {
        ArtMethod::PtrSizedFields ptr_sized_fields_(ptr_sized_fields(), this);
        item = ptr_sized_fields_.data();
    }
    item = item.Ptr() & 0xFFFFFFFFFFFFFFFEULL;

    if (Android::Sdk() >= Android::P) {
        if (dex_file.IsCompactDexFile()) {
            CompactDexFile::CodeItem* compact = reinterpret_cast<CompactDexFile::CodeItem*>(&item);
            compact->DecodeFields();
        } else {
            StandardDexFile::CodeItem* standard = reinterpret_cast<StandardDexFile::CodeItem*>(&item);
            standard->DecodeFields();
        }
    } else {
        StandardDexFile::CodeItem* standard = reinterpret_cast<StandardDexFile::CodeItem*>(&item);
        standard->DecodeFields();
    }
    return item;
}

uint64_t ArtMethod::GetEntryPointFromQuickCompiledCode() {
    return GetEntryPointFromQuickCompiledCodePtrSize(CoreApi::GetPointSize());
}

uint64_t ArtMethod::GetEntryPointFromQuickCompiledCodePtrSize(uint32_t pointer_size) {
    return GetNativePointer(EntryPointFromQuickCompiledCodeOffset(pointer_size), pointer_size);
}

uint32_t ArtMethod::EntryPointFromQuickCompiledCodeOffset(uint32_t pointer_size) {
    return (OFFSET(ArtMethod, ptr_sized_fields_) +
            OFFSET(PtrSizedFields, entry_point_from_quick_compiled_code_) /
            pointer_size * pointer_size);
}

uint64_t ArtMethod::GetNativePointer(uint32_t offset, uint32_t pointer_size) {
    if (pointer_size == static_cast<uint32_t>(PointerSize::k32)) {
        return value32Of(offset);
    } else {
        return value64Of(offset);
    }
}

static const OatFile::OatMethod FindOatMethodFromDexFileFor(ArtMethod& method, bool* found) {
    return OatFile::OatMethod::Invalid();
}

static const OatFile::OatMethod FindOatMethodFor(ArtMethod& method,
                                                 uint32_t pointer_size,
                                                 bool* found) {
    if (method.IsObsolete()) {
        return FindOatMethodFromDexFileFor(method, found);
    }

    mirror::Class& declaring_class = method.GetDeclaringClass();
    uint16_t oat_method_index;
    bool found_virtual = false;
    if (method.IsStatic() || method.IsDirect()) {
        oat_method_index = method.GetMethodIndex();
    } else {
        oat_method_index = declaring_class.NumDirectMethods();
        auto visit_virtual_methods = [&](art::ArtMethod& art_method) -> bool {
            if (method.GetDexMethodIndex() == art_method.GetDexMethodIndex()) {
                found_virtual = true;
                return true;
            }
            oat_method_index++;
            return false;
        };
        Android::ForeachVirtualArtMethods(declaring_class, visit_virtual_methods);
        if (!found_virtual) {
            return OatFile::OatMethod::Invalid();
        }
    }

    OatFile::OatClass oat_class = OatFile::FindOatClass(declaring_class.GetDexFile(),
                                                        declaring_class.GetDexClassDefIndex(),
                                                        found);

    if (!(*found)) {
        return OatFile::OatMethod::Invalid();
    }
    return oat_class.GetOatMethod(oat_method_index);
}

OatQuickMethodHeader ArtMethod::GetOatQuickMethodHeader(uint64_t pc) {
    if (IsRuntimeMethod()) {
        return 0x0;
    }

    Runtime& runtime = Runtime::Current();
    uint64_t existing_entry_point = GetEntryPointFromQuickCompiledCode();
    ClassLinker& class_linker = runtime.GetClassLinker();

    if (existing_entry_point == GetQuickProxyInvokeHandler()) {
        return 0x0;
    }

    if (!class_linker.IsQuickGenericJniStub(existing_entry_point) &&
        !class_linker.IsQuickResolutionStub(existing_entry_point) &&
        !class_linker.IsQuickToInterpreterBridge(existing_entry_point) &&
        existing_entry_point != GetInvokeObsoleteMethodStub()) {
        OatQuickMethodHeader method_header = OatQuickMethodHeader::FromEntryPoint(existing_entry_point);
        if (method_header.Contains(pc)) {
            return method_header;
        }
    }

    if (Android::Sdk() > Android::Q) {
        if (OatQuickMethodHeader::IsNterpPc(pc)) {
            return OatQuickMethodHeader::GetNterpMethodHeader();
        }
    }

    jit::Jit& jit = runtime.GetJit();
    if (jit.Ptr()) {
        jit::JitCodeCache& code_cache = jit.GetCodeCache();
        OatQuickMethodHeader method_header = code_cache.LookupMethodHeader(pc, *this);
        if (method_header.Ptr()
                /* && method_header.Contains(pc)*/) {
            return method_header;
        }
    }

    bool found = false;
    OatFile::OatMethod oat_method = FindOatMethodFor(*this, CoreApi::GetPointSize(), &found);
    if (!found)
        return 0x0;

    uint64_t oat_entry_point = oat_method.GetQuickCode();
    if (!oat_entry_point || class_linker.IsQuickGenericJniStub(oat_entry_point))
        return 0x0;

    OatQuickMethodHeader method_header = OatQuickMethodHeader::FromEntryPoint(oat_entry_point);
    if (IsNative() && !method_header.Contains(pc))
        return 0x0;

    return method_header;
}

const char* ArtMethod::GetShorty(uint32_t* out_length) {
    DexFile& dex_file = GetDexFile();
    dex::MethodId method_id = dex_file.GetMethodId(GetDexMethodIndex());
    return dex_file.GetMethodShorty(method_id, out_length);
}

} //namespace art
