/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License";
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
#include "runtime/cache_helpers.h"

namespace art {

uint64_t CacheHelper::art_jni_dlsym_lookup_stub = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_jni_dlsym_lookup_critical_stub = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_quick_imt_conflict_trampoline = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_quick_to_interpreter_bridge = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_invoke_obsolete_method_stub = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_quick_generic_jni_trampoline = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_quick_proxy_invoke_handler = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_quick_resolution_trampoline = INVALID_ENTRY_POINTER;
uint64_t CacheHelper::art_quick_deoptimize = INVALID_ENTRY_POINTER;

uint64_t CacheHelper::ExecuteNterpImpl = INVALID_ENTRY_POINTER;
OatQuickMethodHeader CacheHelper::NterpMethodHeaderRef = INVALID_ENTRY_POINTER;
api::MemoryRef CacheHelper::NterpWithClinitImplRef = INVALID_ENTRY_POINTER;
api::MemoryRef CacheHelper::NterpImplRef = INVALID_ENTRY_POINTER;

void CacheHelper::EntryPointDump() {
    LOGI("  * art_jni_dlsym_lookup_stub: 0x%lx\n", JniDlsymLookupStub());
    LOGI("  * art_jni_dlsym_lookup_critical_stub: 0x%lx\n", JniDlsymLookupCriticalStub());
    LOGI("  * art_quick_imt_conflict_trampoline: 0x%lx\n", QuickImtConflictStub());
    LOGI("  * art_quick_to_interpreter_bridge: 0x%lx\n", QuickToInterpreterBridge());
    LOGI("  * art_invoke_obsolete_method_stub: 0x%lx\n", InvokeObsoleteMethodStub());
    LOGI("  * art_quick_generic_jni_trampoline: 0x%lx\n", QuickGenericJniStub());
    LOGI("  * art_quick_proxy_invoke_handler: 0x%lx\n", QuickProxyInvokeHandler());
    LOGI("  * art_quick_resolution_trampoline: 0x%lx\n", QuickResolutionStub());
    LOGI("  * art_quick_deoptimize: 0x%lx\n", QuickDeoptimizationEntryPoint());
}

void CacheHelper::NterpDump() {
    Android::OatPrepare();
    LOGI("  * ExecuteNterpImpl: 0x%lx\n", ExecuteNterpImplEntryPoint());
    LOGI("  * art::OatQuickMethodHeader::NterpWithClinitImpl: %lx\n", NterpWithClinitImpl().Ptr());
    LOGI("  * art::OatQuickMethodHeader::NterpImpl: 0x%lx\n", NterpImpl().Ptr());
    LOGI("  * art::OatQuickMethodHeader::NterpMethodHeader: %lx\n", NterpMethodHeader().Ptr());
}

void CacheHelper::Clean() {
    art_jni_dlsym_lookup_stub = INVALID_ENTRY_POINTER;
    art_jni_dlsym_lookup_critical_stub = INVALID_ENTRY_POINTER;
    art_quick_imt_conflict_trampoline = INVALID_ENTRY_POINTER;
    art_quick_to_interpreter_bridge = INVALID_ENTRY_POINTER;
    art_invoke_obsolete_method_stub = INVALID_ENTRY_POINTER;
    art_quick_generic_jni_trampoline = INVALID_ENTRY_POINTER;
    art_quick_proxy_invoke_handler = INVALID_ENTRY_POINTER;
    art_quick_resolution_trampoline = INVALID_ENTRY_POINTER;
    art_quick_deoptimize = INVALID_ENTRY_POINTER;
    ExecuteNterpImpl = INVALID_ENTRY_POINTER;
    NterpMethodHeaderRef = INVALID_ENTRY_POINTER;
    NterpWithClinitImplRef = INVALID_ENTRY_POINTER;
    NterpImplRef = INVALID_ENTRY_POINTER;
}

uint64_t CacheHelper::JniDlsymLookupStub() {
    if (art_jni_dlsym_lookup_stub == INVALID_ENTRY_POINTER) {
        art_jni_dlsym_lookup_stub = Android::DlSym("art_jni_dlsym_lookup_stub");
    }
    return art_jni_dlsym_lookup_stub;
}

uint64_t CacheHelper::JniDlsymLookupCriticalStub() {
    if (art_jni_dlsym_lookup_critical_stub == INVALID_ENTRY_POINTER) {
        art_jni_dlsym_lookup_critical_stub = Android::DlSym("art_jni_dlsym_lookup_critical_stub");
    }
    return art_jni_dlsym_lookup_critical_stub;
}

uint64_t CacheHelper::QuickImtConflictStub() {
    if (art_quick_imt_conflict_trampoline == INVALID_ENTRY_POINTER) {
        art_quick_imt_conflict_trampoline = Android::DlSym("art_quick_imt_conflict_trampoline");
    }
    return art_quick_imt_conflict_trampoline;
}

uint64_t CacheHelper::QuickToInterpreterBridge() {
    if (art_quick_to_interpreter_bridge == INVALID_ENTRY_POINTER) {
        art_quick_to_interpreter_bridge = Android::DlSym("art_quick_to_interpreter_bridge");
    }
    return art_quick_to_interpreter_bridge;
}

uint64_t CacheHelper::InvokeObsoleteMethodStub() {
    if (art_invoke_obsolete_method_stub == INVALID_ENTRY_POINTER) {
        art_invoke_obsolete_method_stub = Android::DlSym("art_invoke_obsolete_method_stub");
    }
    return art_invoke_obsolete_method_stub;
}

uint64_t CacheHelper::QuickGenericJniStub() {
    if (art_quick_generic_jni_trampoline == INVALID_ENTRY_POINTER) {
        art_quick_generic_jni_trampoline = Android::DlSym("art_quick_generic_jni_trampoline");
    }
    return art_quick_generic_jni_trampoline;
}

uint64_t CacheHelper::QuickProxyInvokeHandler() {
    if (art_quick_proxy_invoke_handler == INVALID_ENTRY_POINTER) {
        art_quick_proxy_invoke_handler = Android::DlSym("art_quick_proxy_invoke_handler");
    }
    return art_quick_proxy_invoke_handler;
}

uint64_t CacheHelper::QuickResolutionStub() {
    if (art_quick_resolution_trampoline == INVALID_ENTRY_POINTER) {
        art_quick_resolution_trampoline = Android::DlSym("art_quick_resolution_trampoline");
    }
    return art_quick_resolution_trampoline;
}

uint64_t CacheHelper::QuickDeoptimizationEntryPoint() {
    if (art_quick_deoptimize == INVALID_ENTRY_POINTER) {
        art_quick_deoptimize = Android::DlSym("art_quick_deoptimize");
    }
    return art_quick_deoptimize;
}

uint64_t CacheHelper::ExecuteNterpImplEntryPoint() {
    if (ExecuteNterpImpl == INVALID_ENTRY_POINTER) {
        ExecuteNterpImpl = Android::DlSym(Android::EXECUTE_NTERP_IMPL);
    }
    return ExecuteNterpImpl;
}

OatQuickMethodHeader& CacheHelper::NterpMethodHeader() {
    if (NterpMethodHeaderRef.Ptr())
        return NterpMethodHeaderRef;

    uint64_t entry_point = ExecuteNterpImplEntryPoint();
    if (entry_point) {
        OatQuickMethodHeader::FromEntryPoint(entry_point);
        NterpMethodHeaderRef = OatQuickMethodHeader::FromEntryPoint(entry_point);
        return NterpMethodHeaderRef;
    }

    try {
        api::MemoryRef value = Android::DlSym(Android::NTERP_METHOD_HEADER);
        NterpMethodHeaderRef = value.valueOf();
    } catch(InvalidAddressException e) {
    }

    if (!NterpMethodHeaderRef.Ptr()) {
        if (OatHeader::OatVersion() >= 239) {
            NterpMethodHeaderRef = NterpImpl().Ptr() ?
                                   OatQuickMethodHeader::FromCodePointer(NterpImpl().valueOf())
                                   : 0x0;
        }
    }

    return NterpMethodHeaderRef;
}

api::MemoryRef& CacheHelper::NterpWithClinitImpl() {
    if (!NterpWithClinitImplRef.Ptr()) {
        NterpWithClinitImplRef = Android::DlSym(Android::NTERP_WITH_CLINT_IMPL);
    }
    return NterpWithClinitImplRef;
}

api::MemoryRef& CacheHelper::NterpImpl() {
    if (!NterpImplRef.Ptr()) {
        NterpImplRef = Android::DlSym(Android::NTERP_IMPL);
    }
    return NterpImplRef;
}

} // namespace art
