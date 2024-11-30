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
#include "runtime/jit/jit_memory_region.h"

struct JitMemoryRegion_OffsetTable __JitMemoryRegion_offset__;

namespace art {
namespace jit {

void JitMemoryRegion::Init() {
    Android::RegisterSdkListener(Android::R, art::jit::JitMemoryRegion::Init30);
}

void JitMemoryRegion::Init30() {
    if (CoreApi::Bits() == 64) {
        __JitMemoryRegion_offset__ = {
            .exec_pages_ = 200,
        };
    } else {
        __JitMemoryRegion_offset__ = {
            .exec_pages_ = 108,
        };
    }
}

MemMap& JitMemoryRegion::GetExecPages() {
    if (!exec_pages_cache.Ptr()) {
        exec_pages_cache = exec_pages();
        exec_pages_cache.copyRef(this);
    }
    return exec_pages_cache;
}

} // namespace jit
} // namespace art
