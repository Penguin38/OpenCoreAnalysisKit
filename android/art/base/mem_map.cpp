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

#include "base/mem_map.h"
#include "api/core.h"

struct MemMap_OffsetTable __MemMap_offset__;
struct MemMap_SizeTable __MemMap_size__;

namespace art {

void MemMap::Init() {
    if (CoreApi::Bits() == 64) {
        __MemMap_offset__ = {
            .name_ = 0,
            .begin_ = 24,
            .size_ = 32,
        };

        __MemMap_size__ = {
            .THIS = 72,
        };
    } else {
        __MemMap_offset__ = {
            .name_ = 0,
            .begin_ = 12,
            .size_ = 16,
        };

        __MemMap_size__ = {
            .THIS = 40,
        };
    }
}

const char* MemMap::GetName() {
    if (!name_cache.Ptr()) {
        name_cache = name();
    }
    return name_cache.c_str();
}

} // namespace art
