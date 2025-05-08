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

#include "android.h"
#include "native/android_os_BinderProxy.h"

struct BinderProxyNativeData_OffsetTable __BinderProxyNativeData_offset__;

namespace android {

void BinderProxyNativeData::Init() {
    Android::RegisterSdkListener(Android::P, android::BinderProxyNativeData::Init28);
    Android::RegisterSdkListener(Android::W, android::BinderProxyNativeData::Init36);
}

void BinderProxyNativeData::Init28() {
    if (CoreApi::Bits() == 64) {
        __BinderProxyNativeData_offset__ = {
            .mObject = 0,
            .mOrgue = 8,
        };
    } else {
        __BinderProxyNativeData_offset__ = {
            .mObject = 0,
            .mOrgue = 4,
        };
    }
}

void BinderProxyNativeData::Init36() {
    if (CoreApi::Bits() == 64) {
        __BinderProxyNativeData_offset__ = {
            .mObject = 0,
            .mOrgue = 8,
            .mFrozenStateChangeCallbackList = 16,
        };
    } else {
        __BinderProxyNativeData_offset__ = {
            .mObject = 0,
            .mOrgue = 4,
            .mFrozenStateChangeCallbackList = 8,
        };
    }
}

} // android
