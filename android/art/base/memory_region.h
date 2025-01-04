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

#ifndef ANDROID_ART_BASE_MEMORY_REGION_H_
#define ANDROID_ART_BASE_MEMORY_REGION_H_

#include "api/memory_ref.h"
#include "common/bit.h"
#include "base/globals.h"

namespace art {

class MemoryRegion {
public:
    MemoryRegion() : pointer_(0), size_(0) {}
    MemoryRegion(uint64_t pointer_in, uint64_t size_in) : pointer_(pointer_in), size_(size_in) {}

    uint64_t pointer() { return pointer_; }
    uint64_t size() { return size_; }
    uint64_t size_in_bits() { return size_ * kBitsPerByte; }

    uint64_t begin() { return pointer_; }
    uint64_t end() { return pointer_ + size_; }

    MemoryRegion Subregion(uint64_t offset, uint64_t size_in) {
        return MemoryRegion(begin() + offset, size_in);
    }

    uint32_t LoadUnaligned32(uint64_t offset);
private:
    uint64_t pointer_;
    uint64_t size_;
};

} // namespace art

#endif // ANDROID_ART_BASE_MEMORY_REGION_H_
