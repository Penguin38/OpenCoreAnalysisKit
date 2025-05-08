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
#include "native/android_BpBinder.h"

struct BpBinder_OffsetTable __BpBinder_offset__;

namespace android {

void BpBinder::Init() {
    Android::RegisterSdkListener(Android::R, android::BpBinder::Init30);
    Android::RegisterSdkListener(Android::S, android::BpBinder::Init31);
    Android::RegisterSdkListener(Android::T, android::BpBinder::Init33);
    Android::RegisterSdkListener(Android::U, android::BpBinder::Init34);
    Android::RegisterSdkListener(Android::V, android::BpBinder::Init35);
    Android::RegisterSdkListener(Android::W, android::BpBinder::Init36);
}

void BpBinder::Init30() {
    if (CoreApi::Bits() == 64) {
        __BpBinder_offset__ = {
            .mAlive = 56,
            .mObjects = 72,
            .mDescriptorCache = 128,
            .mTrackedUid = 128,
        };
    } else {
        __BpBinder_offset__ = {
            .mAlive = 16,
            .mObjects = 28,
            .mDescriptorCache = 52,
            .mTrackedUid = 56,
        };
    }
}

void BpBinder::Init31() {
    if (CoreApi::Bits() == 64) {
        __BpBinder_offset__ = {
            .mAlive = 88,
            .mObjects = 104,
            .mDescriptorCache = 144,
            .mTrackedUid = 152,
        };
    } else {
        __BpBinder_offset__ = {
            .mAlive = 28,
            .mObjects = 40,
            .mDescriptorCache = 60,
            .mTrackedUid = 64,
        };
    }
}


void BpBinder::Init33() {
    if (CoreApi::Bits() == 64) {
        __BpBinder_offset__ = {
            .mAlive = 80,
            .mObjects = 96,
            .mDescriptorCache = 120,
            .mTrackedUid = 128,
        };
    } else {
        __BpBinder_offset__ = {
            .mAlive = 36,
            .mObjects = 48,
            .mDescriptorCache = 60,
            .mTrackedUid = 64,
        };
    }
}

void BpBinder::Init34() {
    if (CoreApi::Bits() == 64) {
        __BpBinder_offset__ = {
            .mAlive = 80,
            .mObjects = 96,
            .mDescriptorCache = 120,
            .mTrackedUid = 128,
        };
    } else {
        __BpBinder_offset__ = {
            .mAlive = 36,
            .mObjects = 48,
            .mDescriptorCache = 60,
            .mTrackedUid = 64,
        };
    }
}

void BpBinder::Init35() {
    if (CoreApi::Bits() == 64) {
        __BpBinder_offset__ = {
            .mAlive = 80,
            .mObjects = 96,
            .mDescriptorCache = 120,
            .mTrackedUid = 128,
        };
    } else {
        __BpBinder_offset__ = {
            .mAlive = 36,
            .mObjects = 48,
            .mDescriptorCache = 60,
            .mTrackedUid = 64,
        };
    }
}

void BpBinder::Init36() {
    if (CoreApi::Bits() == 64) {
        __BpBinder_offset__ = {
            .mAlive = 80,
            .mObjects = 96,
            .mDescriptorCache = 120,
            .mTrackedUid = 128,
        };
    } else {
        __BpBinder_offset__ = {
            .mAlive = 36,
            .mObjects = 48,
            .mDescriptorCache = 60,
            .mTrackedUid = 64,
        };
    }
}

} // android
