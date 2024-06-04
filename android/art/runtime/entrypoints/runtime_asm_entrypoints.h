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

// very few
static uint64_t INVALID_ENTRY_POINTER = 0xFFFFFFFFFFFFFFFFUL;
static uint64_t art_jni_dlsym_lookup_stub = INVALID_ENTRY_POINTER;
static uint64_t art_jni_dlsym_lookup_critical_stub = INVALID_ENTRY_POINTER;
static uint64_t art_quick_imt_conflict_trampoline = INVALID_ENTRY_POINTER;
static uint64_t art_quick_to_interpreter_bridge = INVALID_ENTRY_POINTER;
static uint64_t art_invoke_obsolete_method_stub = INVALID_ENTRY_POINTER;
static uint64_t art_quick_generic_jni_trampoline = INVALID_ENTRY_POINTER;
static uint64_t art_quick_proxy_invoke_handler = INVALID_ENTRY_POINTER;
static uint64_t art_quick_resolution_trampoline = INVALID_ENTRY_POINTER;
static uint64_t art_quick_deoptimize = INVALID_ENTRY_POINTER;

static inline uint64_t GetJniDlsymLookupStub() {
    return art_jni_dlsym_lookup_stub;
}

static inline uint64_t GetJniDlsymLookupCriticalStub() {
    return art_jni_dlsym_lookup_critical_stub;
}

static inline uint64_t GetQuickImtConflictStub() {
    return art_quick_imt_conflict_trampoline;
}

static inline uint64_t GetQuickToInterpreterBridge() {
    return art_quick_to_interpreter_bridge;
}

static inline uint64_t GetInvokeObsoleteMethodStub() {
    return art_invoke_obsolete_method_stub;
}

static inline uint64_t GetQuickGenericJniStub() {
    return art_quick_generic_jni_trampoline;
}

static inline uint64_t GetQuickProxyInvokeHandler() {
    return art_quick_proxy_invoke_handler;
}

static inline uint64_t GetQuickResolutionStub() {
    return art_quick_resolution_trampoline;
}

static inline uint64_t GetQuickDeoptimizationEntryPoint() {
    return art_quick_deoptimize;
}

#endif // ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_
