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

#include "base/memory_region.h"
#include <type_traits>

namespace art {

uint32_t MemoryRegion::LoadUnaligned32(uint64_t offset) {
    typedef typename std::make_unsigned<uint32_t>::type U;
    U equivalent_unsigned_integer_value = 0;
    api::MemoryRef ref = begin();
    for (size_t i = 0; i < sizeof(U); ++i) {
        equivalent_unsigned_integer_value += ref.value8Of(offset + i) << (i * kBitsPerByte);
    }
    return reinterpret_cast<uint32_t>(equivalent_unsigned_integer_value);
}

} // namespace art
