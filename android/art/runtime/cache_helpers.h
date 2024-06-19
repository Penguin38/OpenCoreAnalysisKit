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

#ifndef ANDROID_ART_RUNTIME_CACHE_HELPERS_H_
#define ANDROID_ART_RUNTIME_CACHE_HELPERS_H_

#include "runtime/oat_quick_method_header.h"
#include <stdint.h>

namespace art {

class CacheHelper {
public:
    static constexpr uint64_t INVALID_ENTRY_POINTER = 0;
    static void EntryPointDump();
    static void NterpDump();
    static void Clean();

    static uint64_t JniDlsymLookupStub();
    static uint64_t JniDlsymLookupCriticalStub();
    static uint64_t QuickImtConflictStub();
    static uint64_t QuickToInterpreterBridge();
    static uint64_t InvokeObsoleteMethodStub();
    static uint64_t QuickGenericJniStub();
    static uint64_t QuickProxyInvokeHandler();
    static uint64_t QuickResolutionStub();
    static uint64_t QuickDeoptimizationEntryPoint();
    static uint64_t ExecuteNterpImplEntryPoint();
    static OatQuickMethodHeader& NterpMethodHeader();
    static api::MemoryRef& NterpWithClinitImpl();
    static api::MemoryRef& NterpImpl();
private:
    // entry pointers
    static uint64_t art_jni_dlsym_lookup_stub;
    static uint64_t art_jni_dlsym_lookup_critical_stub;
    static uint64_t art_quick_imt_conflict_trampoline;
    static uint64_t art_quick_to_interpreter_bridge;
    static uint64_t art_invoke_obsolete_method_stub;
    static uint64_t art_quick_generic_jni_trampoline;
    static uint64_t art_quick_proxy_invoke_handler;
    static uint64_t art_quick_resolution_trampoline;
    static uint64_t art_quick_deoptimize;

    // nterp
    static uint64_t ExecuteNterpImpl;
    static OatQuickMethodHeader NterpMethodHeaderRef;
    static api::MemoryRef NterpWithClinitImplRef;
    static api::MemoryRef NterpImplRef;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_CACHE_HELPERS_H_
