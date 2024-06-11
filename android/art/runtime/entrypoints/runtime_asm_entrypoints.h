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

#ifndef ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_
#define ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_

#include <stdint.h>

namespace art {

class EntryPoints {
public:
    static constexpr uint64_t INVALID_ENTRY_POINTER = 0;
    static uint64_t art_jni_dlsym_lookup_stub;
    static uint64_t art_jni_dlsym_lookup_critical_stub;
    static uint64_t art_quick_imt_conflict_trampoline;
    static uint64_t art_quick_to_interpreter_bridge;
    static uint64_t art_invoke_obsolete_method_stub;
    static uint64_t art_quick_generic_jni_trampoline;
    static uint64_t art_quick_proxy_invoke_handler;
    static uint64_t art_quick_resolution_trampoline;
    static uint64_t art_quick_deoptimize;
    static uint64_t ExecuteNterpImpl;

    static void Init();
    static void Dump();
};

static inline uint64_t GetJniDlsymLookupStub() {
    return EntryPoints::art_jni_dlsym_lookup_stub;
}

static inline uint64_t GetJniDlsymLookupCriticalStub() {
    return EntryPoints::art_jni_dlsym_lookup_critical_stub;
}

static inline uint64_t GetQuickImtConflictStub() {
    return EntryPoints::art_quick_imt_conflict_trampoline;
}

static inline uint64_t GetQuickToInterpreterBridge() {
    return EntryPoints::art_quick_to_interpreter_bridge;
}

static inline uint64_t GetInvokeObsoleteMethodStub() {
    return EntryPoints::art_invoke_obsolete_method_stub;
}

static inline uint64_t GetQuickGenericJniStub() {
    return EntryPoints::art_quick_generic_jni_trampoline;
}

static inline uint64_t GetQuickProxyInvokeHandler() {
    return EntryPoints::art_quick_proxy_invoke_handler;
}

static inline uint64_t GetQuickResolutionStub() {
    return EntryPoints::art_quick_resolution_trampoline;
}

static inline uint64_t GetQuickDeoptimizationEntryPoint() {
    return EntryPoints::art_quick_deoptimize;
}

static inline uint64_t GetExecuteNterpImplEntryPoint() {
    return EntryPoints::ExecuteNterpImpl;
}

} // namespace art

#endif // ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_
