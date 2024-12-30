/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
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


#include "api/core.h"
#include "android.h"
#include "runtime/mirror/object.h"
#include "runtime/handle_scope.h"

struct HandleScope_SizeTable __HandleScope_size__;

namespace art {

void HandleScope::Init() {
    Android::RegisterSdkListener(Android::M, art::HandleScope::Init23);
    Android::RegisterSdkListener(Android::U, art::HandleScope::Init34);
}

void HandleScope::Init23() {
    if (CoreApi::Bits() == 64) {
        __HandleScope_size__ = {
            .THIS = 12,
        };
    } else {
        __HandleScope_size__ = {
            .THIS = 8,
        };
    }
}

void HandleScope::Init34() {
    if (CoreApi::Bits() == 64) {
        __HandleScope_size__ = {
            .THIS = 16,
        };
    } else {
        __HandleScope_size__ = {
            .THIS = 12,
        };
    }
}

uint32_t HandleScope::SizeOf(uint32_t num_references) {
    uint32_t header_size = SIZEOF(HandleScope);
    uint32_t data_size = SIZEOF(Object) * num_references;
    return header_size + data_size;
}

} //namespace art
