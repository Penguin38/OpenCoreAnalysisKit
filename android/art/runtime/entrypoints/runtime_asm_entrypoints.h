/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License";
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

#ifndef ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_
#define ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_

#include "runtime/cache_helpers.h"
#include <stdint.h>

namespace art {

static inline uint64_t GetJniDlsymLookupStub() {
    return CacheHelper::JniDlsymLookupStub();
}

static inline uint64_t GetJniDlsymLookupCriticalStub() {
    return CacheHelper::JniDlsymLookupCriticalStub();
}

static inline uint64_t GetQuickImtConflictStub() {
    return CacheHelper::QuickImtConflictStub();
}

static inline uint64_t GetQuickToInterpreterBridge() {
    return CacheHelper::QuickToInterpreterBridge();
}

static inline uint64_t GetInvokeObsoleteMethodStub() {
    return CacheHelper::InvokeObsoleteMethodStub();
}

static inline uint64_t GetQuickGenericJniStub() {
    return CacheHelper::QuickGenericJniStub();
}

static inline uint64_t GetQuickProxyInvokeHandler() {
    return CacheHelper::QuickProxyInvokeHandler();
}

static inline uint64_t GetQuickResolutionStub() {
    return CacheHelper::QuickResolutionStub();
}

static inline uint64_t GetQuickDeoptimizationEntryPoint() {
    return CacheHelper::QuickDeoptimizationEntryPoint();
}

static inline uint64_t GetExecuteNterpImplEntryPoint() {
    return CacheHelper::ExecuteNterpImplEntryPoint();
}

} // namespace art

#endif // ANDROID_ART_RUNTIME_ENTRYPOINTS_RUNTIME_ASM_ENTRYPOINTS_H_
