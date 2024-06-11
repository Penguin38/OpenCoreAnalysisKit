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
#include "runtime/entrypoints/runtime_asm_entrypoints.h"

namespace art {

uint64_t EntryPoints::art_jni_dlsym_lookup_stub = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_jni_dlsym_lookup_critical_stub = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_quick_imt_conflict_trampoline = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_quick_to_interpreter_bridge = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_invoke_obsolete_method_stub = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_quick_generic_jni_trampoline = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_quick_proxy_invoke_handler = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_quick_resolution_trampoline = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::art_quick_deoptimize = INVALID_ENTRY_POINTER;
uint64_t EntryPoints::ExecuteNterpImpl = INVALID_ENTRY_POINTER;

void EntryPoints::Init() {
    art_jni_dlsym_lookup_stub = Android::SearchSymbol("art_jni_dlsym_lookup_stub");
    art_jni_dlsym_lookup_critical_stub = Android::SearchSymbol("art_jni_dlsym_lookup_critical_stub");
    art_quick_imt_conflict_trampoline = Android::SearchSymbol("art_quick_imt_conflict_trampoline");
    art_quick_to_interpreter_bridge = Android::SearchSymbol("art_quick_to_interpreter_bridge");
    art_invoke_obsolete_method_stub = Android::SearchSymbol("art_invoke_obsolete_method_stub");
    art_quick_generic_jni_trampoline = Android::SearchSymbol("art_quick_generic_jni_trampoline");
    art_quick_proxy_invoke_handler = Android::SearchSymbol("art_quick_proxy_invoke_handler");
    art_quick_resolution_trampoline = Android::SearchSymbol("art_quick_resolution_trampoline");
    art_quick_deoptimize = Android::SearchSymbol("art_quick_deoptimize");
    ExecuteNterpImpl = Android::SearchSymbol(Android::EXECUTE_NTERP_IMPL);
}

void EntryPoints::Dump() {
    LOGI("  * art_jni_dlsym_lookup_stub: 0x%lx\n", art_jni_dlsym_lookup_stub);
    LOGI("  * art_jni_dlsym_lookup_critical_stub: 0x%lx\n", art_jni_dlsym_lookup_critical_stub);
    LOGI("  * art_quick_imt_conflict_trampoline: 0x%lx\n", art_quick_imt_conflict_trampoline);
    LOGI("  * art_quick_to_interpreter_bridge: 0x%lx\n", art_quick_to_interpreter_bridge);
    LOGI("  * art_invoke_obsolete_method_stub: 0x%lx\n", art_invoke_obsolete_method_stub);
    LOGI("  * art_quick_generic_jni_trampoline: 0x%lx\n", art_quick_generic_jni_trampoline);
    LOGI("  * art_quick_proxy_invoke_handler: 0x%lx\n", art_quick_proxy_invoke_handler);
    LOGI("  * art_quick_resolution_trampoline: 0x%lx\n", art_quick_resolution_trampoline);
    LOGI("  * art_quick_deoptimize: 0x%lx\n", art_quick_deoptimize);

    LOGI("\n  * ExecuteNterpImpl: 0x%lx\n", ExecuteNterpImpl);
}

} // namespace art
