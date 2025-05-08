/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef ANDROID_SDK_NATIVE_ANDROID_OS_BINDERPROXY_H_
#define ANDROID_SDK_NATIVE_ANDROID_OS_BINDERPROXY_H_

#include "api/memory_ref.h"

struct BinderProxyNativeData_OffsetTable {
    uint32_t mObject;
    uint32_t mOrgue;
    uint32_t mFrozenStateChangeCallbackList;
};

extern struct BinderProxyNativeData_OffsetTable __BinderProxyNativeData_offset__;

namespace android {

class BinderProxyNativeData : public api::MemoryRef {
public:
    BinderProxyNativeData(uint64_t v) : api::MemoryRef(v) {}
    BinderProxyNativeData(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    BinderProxyNativeData(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    BinderProxyNativeData(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    BinderProxyNativeData(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static void Init28();
    static void Init36();

    inline uint64_t mObject() { return VALUEOF(BinderProxyNativeData, mObject); }
    inline uint64_t mOrgue() { return VALUEOF(BinderProxyNativeData, mOrgue); }
    inline uint64_t mFrozenStateChangeCallbackList() { return VALUEOF(BinderProxyNativeData, mFrozenStateChangeCallbackList); }
};

} // android

#endif // ANDROID_SDK_NATIVE_ANDROID_OS_BINDERPROXY_H_
