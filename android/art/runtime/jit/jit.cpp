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
#include "runtime/jit/jit.h"

struct Jit_OffsetTable __Jit_offset__;

namespace art {
namespace jit {

void Jit::Init26() {
    if (CoreApi::Bits() == 64) {
        __Jit_offset__ = {
            .code_cache_ = 368,
        };
    } else {
        __Jit_offset__ = {
            .code_cache_ = 248,
        };
    }
}

void Jit::Init28() {
    if (CoreApi::Bits() == 64) {
        __Jit_offset__ = {
            .code_cache_ = 352,
        };
    } else {
        __Jit_offset__ = {
            .code_cache_ = 240,
        };
    }
}

void Jit::Init29() {
    if (CoreApi::Bits() == 64) {
        __Jit_offset__ = {
            .code_cache_ = 8,
        };
    } else {
        __Jit_offset__ = {
            .code_cache_ = 4,
        };
    }
}

JitCodeCache& Jit::GetCodeCache() {
    if (!code_cache_cache.Ptr()) {
        code_cache_cache = code_cache();
        code_cache_cache.copyRef(this);
        code_cache_cache.Prepare(false);
    }
    return code_cache_cache;
}

} // namespace jit
} // namespace art
