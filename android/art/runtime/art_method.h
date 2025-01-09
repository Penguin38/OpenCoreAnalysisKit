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

#ifndef ANDROID_ART_RUNTIME_ART_METHOD_H_
#define ANDROID_ART_RUNTIME_ART_METHOD_H_

#include "api/memory_ref.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/dex_cache.h"
#include "runtime/oat_quick_method_header.h"
#include "dex/modifiers.h"
#include "dex/dex_file_types.h"

struct ArtMethod_OffsetTable {
    uint32_t declaring_class_;
    uint32_t access_flags_;
    uint32_t dex_code_item_offset_;
    uint32_t dex_method_index_;
    uint32_t method_index_;
    uint32_t hotness_count_;
    uint32_t imt_index_;
    uint32_t ptr_sized_fields_;
};

struct ArtMethod_SizeTable {
    uint32_t THIS;
};

extern struct ArtMethod_OffsetTable __ArtMethod_offset__;
extern struct ArtMethod_SizeTable __ArtMethod_size__;

struct PtrSizedFields_OffsetTable {
    uint32_t dex_cache_resolved_methods_;
    uint32_t data_;
    uint32_t entry_point_from_quick_compiled_code_;
};

extern struct PtrSizedFields_OffsetTable __PtrSizedFields_offset__;

namespace art {

class ArtMethod : public api::MemoryRef {
public:
    static constexpr uint32_t kRuntimeMethodDexMethodIndex = 0xFFFFFFFF;

    ArtMethod(uint64_t v) : api::MemoryRef(v) {}
    ArtMethod(api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ArtMethod(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ArtMethod(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(ArtMethod& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(ArtMethod& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    static void Init23();
    static void Init24();
    static void Init26();
    static void Init28();
    static void Init31();
    inline uint32_t declaring_class() { return value32Of(OFFSET(ArtMethod, declaring_class_)); }
    inline uint32_t access_flags() { return value32Of(OFFSET(ArtMethod, access_flags_)); }
    inline uint32_t dex_code_item_offset() { return value32Of(OFFSET(ArtMethod, dex_code_item_offset_)); }
    inline uint32_t dex_method_index() { return value32Of(OFFSET(ArtMethod, dex_method_index_)); }
    inline uint16_t method_index() { return value16Of(OFFSET(ArtMethod, method_index_)); }
    inline uint16_t hotness_count() { return value16Of(OFFSET(ArtMethod, hotness_count_)); }
    inline uint16_t imt_index() { return value16Of(OFFSET(ArtMethod, imt_index_)); }
    inline uint64_t ptr_sized_fields() { return Ptr() + OFFSET(ArtMethod, ptr_sized_fields_); }

    class PtrSizedFields : public api::MemoryRef {
    public:
        PtrSizedFields(uint64_t v) : api::MemoryRef(v) {}
        PtrSizedFields(api::MemoryRef& ref) : api::MemoryRef(ref) {}
        PtrSizedFields(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        PtrSizedFields(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init23();
        static void Init24();
        static void Init26();
        static void Init28();
        inline uint64_t data() { return VALUEOF(PtrSizedFields, data_); }
        inline uint64_t entry_point_from_quick_compiled_code() { return VALUEOF(PtrSizedFields, entry_point_from_quick_compiled_code_); }
    };

    mirror::Class& GetDeclaringClass();
    inline uint32_t GetAccessFlags() { return access_flags(); }
    inline uint32_t GetDexMethodIndex() { return dex_method_index(); }
    inline uint16_t GetMethodIndex() { return method_index(); }
    inline uint16_t GetCounter() { return hotness_count(); }
    inline uint16_t GetImtIndex() { return imt_index(); }

    // Returns true if the method is declared public.
    inline bool IsPublic() {
        return IsPublic(GetAccessFlags());
    }

    static bool IsPublic(uint32_t access_flags) {
        return (access_flags & kAccPublic) != 0;
    }

    // Returns true if the method is declared private.
    inline bool IsPrivate() {
        return IsPrivate(GetAccessFlags());
    }

    static bool IsPrivate(uint32_t access_flags) {
        return (access_flags & kAccPrivate) != 0;
    }

    // Returns true if the method is declared static.
    inline bool IsStatic() {
        return IsStatic(GetAccessFlags());
    }

    static bool IsStatic(uint32_t access_flags) {
        return (access_flags & kAccStatic) != 0;
    }

    // Returns true if the method is a constructor according to access flags.
    inline bool IsConstructor() {
        return IsConstructor(GetAccessFlags());
    }

    static bool IsConstructor(uint32_t access_flags) {
        return (access_flags & kAccConstructor) != 0;
    }

    // Returns true if the method is a class initializer according to access flags.
    inline bool IsClassInitializer() {
        return IsClassInitializer(GetAccessFlags());
    }

    static bool IsClassInitializer(uint32_t access_flags) {
        return IsConstructor(access_flags) && IsStatic(access_flags);
    }

    // Returns true if the method is static, private, or a constructor.
    inline bool IsDirect() {
        return IsDirect(GetAccessFlags());
    }

    static bool IsDirect(uint32_t access_flags) {
        constexpr uint32_t direct = kAccStatic | kAccPrivate | kAccConstructor;
        return (access_flags & direct) != 0;
    }

    // Returns true if the method is declared synchronized.
    inline bool IsSynchronized() {
        return IsSynchronized(GetAccessFlags());
    }

    static bool IsSynchronized(uint32_t access_flags) {
        constexpr uint32_t synchonized = kAccSynchronized | kAccDeclaredSynchronized;
        return (access_flags & synchonized) != 0;
    }

    // Returns true if the method is declared final.
    inline bool IsFinal() {
        return IsFinal(GetAccessFlags());
    }

    static bool IsFinal(uint32_t access_flags) {
        return (access_flags & kAccFinal) != 0;
    }

    // Returns true if the method is an intrinsic.
    inline bool IsIntrinsic() {
        return IsIntrinsic(GetAccessFlags());
    }

    static bool IsIntrinsic(uint32_t access_flags) {
        return (access_flags & kAccIntrinsic) != 0;
    }

    // Returns true if the method is a copied method.
    inline bool IsCopied() {
        return IsCopied(GetAccessFlags());
    }

    static bool IsCopied(uint32_t access_flags) {
        bool copied = (access_flags & (kAccIntrinsic | kAccCopied)) == kAccCopied;
        return copied;
    }

    inline bool IsMiranda() {
        return IsMiranda(GetAccessFlags());
    }

    static bool IsMiranda(uint32_t access_flags) {
        // Miranda methods are marked as copied and abstract but not default.
        // We need to check the kAccIntrinsic too, see `IsCopied()`.
        static constexpr uint32_t kMask = kAccIntrinsic | kAccCopied | kAccAbstract | kAccDefault;
        static constexpr uint32_t kValue = kAccCopied | kAccAbstract;
        return (access_flags & kMask) == kValue;
    }

    // A default conflict method is a special sentinel method that stands for a conflict between
    // multiple default methods. It cannot be invoked, throwing an IncompatibleClassChangeError
    // if one attempts to do so.
    inline bool IsDefaultConflicting() {
        return IsDefaultConflicting(GetAccessFlags());
    }

    static bool IsDefaultConflicting(uint32_t access_flags) {
        // Default conflct methods are marked as copied, abstract and default.
        // We need to check the kAccIntrinsic too, see `IsCopied()`.
        static constexpr uint32_t kMask = kAccIntrinsic | kAccCopied | kAccAbstract | kAccDefault;
        static constexpr uint32_t kValue = kAccCopied | kAccAbstract | kAccDefault;
        return (access_flags & kMask) == kValue;
    }

    // Returns true if invoking this method will not throw an AbstractMethodError or
    // IncompatibleClassChangeError.
    inline bool IsInvokable() {
        return IsInvokable(GetAccessFlags());
    }

    static bool IsInvokable(uint32_t access_flags) {
        // Default conflicting methods are marked with `kAccAbstract` (as well as `kAccCopied`
        // and `kAccDefault`) but they are not considered abstract, see `IsAbstract()`.
        return (access_flags & kAccAbstract) == 0;
    }

    // Returns true if the method is marked as pre-compiled.
    inline bool IsPreCompiled() {
        return IsPreCompiled(GetAccessFlags());
    }

    static bool IsPreCompiled(uint32_t access_flags) {
        // kAccCompileDontBother and kAccPreCompiled overlap with kAccIntrinsicBits.
        static constexpr uint32_t kMask = kAccIntrinsic | kAccCompileDontBother | kAccPreCompiled;
        static constexpr uint32_t kValue = kAccCompileDontBother | kAccPreCompiled;
        return (access_flags & kMask) == kValue;
    }

    // Returns true if the method resides in shared memory.
    inline bool IsMemorySharedMethod() {
        return IsMemorySharedMethod(GetAccessFlags());
    }

    static bool IsMemorySharedMethod(uint32_t access_flags) {
        return (access_flags & kAccMemorySharedMethod) != 0;
    }

    // Returns true if the method can be compiled.
    inline bool IsCompilable() {
        return IsCompilable(GetAccessFlags());
    }

    static bool IsCompilable(uint32_t access_flags) {
        if (IsIntrinsic(access_flags)) {
            // kAccCompileDontBother overlaps with kAccIntrinsicBits.
            return true;
        }
        if (IsPreCompiled(access_flags)) {
            return true;
        }
        return (access_flags & kAccCompileDontBother) == 0;
    }

    // This is set by the class linker.
    inline bool IsDefault() {
        return IsDefault(GetAccessFlags());
    }

    static bool IsDefault(uint32_t access_flags) {
        static_assert((kAccDefault & (kAccIntrinsic | kAccIntrinsicBits)) == 0,
                "kAccDefault conflicts with intrinsic modifier");
        return (access_flags & kAccDefault) != 0;
    }

    // Returns true if the method is obsolete.
    inline bool IsObsolete() {
        return IsObsolete(GetAccessFlags());
    }

    static bool IsObsolete(uint32_t access_flags) {
        return (access_flags & kAccObsoleteMethod) != 0;
    }

    // Returns true if the method is native.
    inline bool IsNative() {
        return IsNative(GetAccessFlags());
    }

    static bool IsNative(uint32_t access_flags) {
        return (access_flags & kAccNative) != 0;
    }

    // Checks to see if the method was annotated with @dalvik.annotation.optimization.FastNative.
    inline bool IsFastNative() {
        return IsFastNative(GetAccessFlags());
    }

    static bool IsFastNative(uint32_t access_flags) {
        // The presence of the annotation is checked by ClassLinker and recorded in access flags.
        // The kAccFastNative flag value is used with a different meaning for non-native methods,
        // so we need to check the kAccNative flag as well.
        constexpr uint32_t mask = kAccFastNative | kAccNative;
        return (access_flags & mask) == mask;
    }

    // Checks to see if the method was annotated with @dalvik.annotation.optimization.CriticalNative.
    inline bool IsCriticalNative() {
        return IsCriticalNative(GetAccessFlags());
    }

    static bool IsCriticalNative(uint32_t access_flags) {
        // The presence of the annotation is checked by ClassLinker and recorded in access flags.
        // The kAccCriticalNative flag value is used with a different meaning for non-native methods,
        // so we need to check the kAccNative flag as well.
        constexpr uint32_t mask = kAccCriticalNative | kAccNative;
        return (access_flags & mask) == mask;
    }

    // Returns true if the method is managed (not native).
    inline bool IsManaged() {
        return IsManaged(GetAccessFlags());
    }

    static bool IsManaged(uint32_t access_flags) {
        return !IsNative(access_flags);
    }

    // Returns true if the method is managed (not native) and invokable.
    inline bool IsManagedAndInvokable() {
        return IsManagedAndInvokable(GetAccessFlags());
    }

    static bool IsManagedAndInvokable(uint32_t access_flags) {
        return IsManaged(access_flags) && IsInvokable(access_flags);
    }

    // Returns true if the method is abstract.
    inline bool IsAbstract() {
        return IsAbstract(GetAccessFlags());
    }

    static bool IsAbstract(uint32_t access_flags) {
        // Default confliciting methods have `kAccAbstract` set but they are not actually abstract.
        return (access_flags & kAccAbstract) != 0 && !IsDefaultConflicting(access_flags);
    }

    // Returns true if the method is declared synthetic.
    inline bool IsSynthetic() {
        return IsSynthetic(GetAccessFlags());
    }

    static bool IsSynthetic(uint32_t access_flags) {
        return (access_flags & kAccSynthetic) != 0;
    }

    // Returns true if the method is declared varargs.
    inline bool IsVarargs() {
        return IsVarargs(GetAccessFlags());
    }

    static bool IsVarargs(uint32_t access_flags) {
        return (access_flags & kAccVarargs) != 0;
    }

    inline bool IsProxyMethod() {
        return GetDeclaringClass().IsProxyClass();
    }

    // Returns true if the method has previously been warm.
    inline bool PreviouslyWarm() {
        return PreviouslyWarm(GetAccessFlags());
    }

    static bool PreviouslyWarm(uint32_t access_flags) {
        // kAccPreviouslyWarm overlaps with kAccIntrinsicBits. Return true for intrinsics.
        constexpr uint32_t mask = kAccPreviouslyWarm | kAccIntrinsic;
        return (access_flags & mask) != 0u;
    }

    // Should this method be run in the interpreter and count locks (e.g., failed structured-
    // locking verification)?
    inline bool MustCountLocks() {
        return MustCountLocks(GetAccessFlags());
    }

    static bool MustCountLocks(uint32_t access_flags) {
        if (IsIntrinsic(access_flags)) {
            return false;
        }
        return (access_flags & kAccMustCountLocks) != 0;
    }

    // Returns true if the method is using the nterp entrypoint fast path.
    inline bool HasNterpEntryPointFastPathFlag() {
        return HasNterpEntryPointFastPathFlag(GetAccessFlags());
    }

    static bool HasNterpEntryPointFastPathFlag(uint32_t access_flags) {
        constexpr uint32_t mask = kAccNative | kAccNterpEntryPointFastPathFlag;
        return (access_flags & mask) == kAccNterpEntryPointFastPathFlag;
    }

    inline bool IsRuntimeMethod() {
        return dex_method_index() == kRuntimeMethodDexMethodIndex;
    }

    mirror::DexCache& GetDexCache();
    DexFile& GetDexFile();
    dex::TypeIndex GetReturnTypeIndex();
    const char* GetReturnTypeDescriptor();
    std::string PrettyReturnTypeDescriptor();
    const char* GetName();
    const char* GetRuntimeMethodName();
    std::string PrettyParameters();
    std::string ColorPrettyMethodOnlyNP();
    std::string ColorPrettyMethodSimple();
    std::string ColorPrettyMethod();
    bool HasCodeItem();
    dex::CodeItem GetCodeItem();
    uint64_t GetEntryPointFromQuickCompiledCode();
    uint64_t GetEntryPointFromQuickCompiledCodePtrSize(uint32_t pointer_size);
    uint32_t EntryPointFromQuickCompiledCodeOffset(uint32_t pointer_size);
    uint64_t GetNativePointer(uint32_t offset, uint32_t pointer_size);
    OatQuickMethodHeader GetOatQuickMethodHeader(uint64_t pc);
    inline const char* GetShorty() {
        uint32_t unused_length;
        return GetShorty(&unused_length);
    }
    const char* GetShorty(uint32_t* out_length);
private:
    // quick memoryref cache
    mirror::Class declaring_class_cache = 0x0;

    // empty
    mirror::DexCache obsolete_dex_caches = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_ART_METHOD_H_
