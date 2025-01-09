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

#include "api/core.h"
#include "android.h"
#include "runtime/jni/jni_env_ext.h"
#include <string.h>

struct JNIEnvExt_OffsetTable __JNIEnvExt_offset__;

namespace art {

void JNIEnvExt::Init() {
    Android::RegisterSdkListener(Android::M, art::JNIEnvExt::Init23);
    Android::RegisterSdkListener(Android::V, art::JNIEnvExt::Init35);
}

void JNIEnvExt::Init23() {
    if (CoreApi::Bits() == 64) {
        __JNIEnvExt_offset__ = {
            .functions = 0,
            .self_ = 8,
            .vm_ = 16,
            .locals_ = 32,
        };
    } else {
        __JNIEnvExt_offset__ = {
            .functions = 0,
            .self_ = 4,
            .vm_ = 8,
            .locals_ = 16,
        };
    }
}

void JNIEnvExt::Init35() {
    if (CoreApi::Bits() == 64) {
        __JNIEnvExt_offset__ = {
            .functions = 0,
            .self_ = 8,
            .vm_ = 16,
            .locals_ = 24,
        };
    } else {
        __JNIEnvExt_offset__ = {
            .functions = 0,
            .self_ = 4,
            .vm_ = 8,
            .locals_ = 12,
        };
    }
}

mirror::Object JNIEnvExt::Decode(uint64_t uref) {
    IndirectRefKind kind = IndirectReferenceTable::DecodeIndirectRefKind(uref);
    if (kind == IndirectRefKind::kLocal)
        return DecodeLocal(uref);
    return 0x0;
}

mirror::Object JNIEnvExt::DecodeLocal(uint64_t uref) {
    if (Android::Sdk() >= Android::U)
        return GetLocalsTable_v34().DecodeReference(uref);
    else return GetLocalsTable().DecodeReference(IndirectReferenceTable::DecodeIndex(uref));
}

} //namespace art
