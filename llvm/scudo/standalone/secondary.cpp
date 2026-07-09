/*
 * Copyright (C) 2026-present, Guanyou.Chen. All rights reserved.
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
#include "scudo/standalone/secondary.h"

struct scudo_LargeBlock_Header_OffsetTable __scudo_LargeBlock_Header_offset__;
struct scudo_LargeBlock_Header_SizeTable __scudo_LargeBlock_Header_size__;

namespace scudo {
namespace LargeBlock {

void Header::Init() {
    if (CoreApi::Bits() == 64) {
        __scudo_LargeBlock_Header_offset__ = {
            .__Prev__ = 0,
            .__Next__ = 8,
            .__CommitBase__ = 16,
            .__CommitSize__ = 24,
            .__MapBase__ = 32,
            .__MapCapacity__ = 40,
        };

        __scudo_LargeBlock_Header_size__ = {
            .THIS = 48,
        };
    } else {
        __scudo_LargeBlock_Header_offset__ = {
            .__Prev__ = 0,
            .__Next__ = 4,
            .__CommitBase__ = 8,
            .__CommitSize__ = 12,
            .__MapBase__ = 16,
            .__MapCapacity__ = 20,
        };

        __scudo_LargeBlock_Header_size__ = {
            .THIS = 32,
        };
    }
}

} // namespace LargeBlock
} // namespace scudo
