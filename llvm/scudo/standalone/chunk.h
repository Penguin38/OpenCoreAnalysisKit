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

#ifndef LLVM_SCUDO_CHUNK_H_
#define LLVM_SCUDO_CHUNK_H_

#include <stdint.h>
#include <sys/types.h>

namespace scudo {
namespace Chunk {

enum Origin : uint8_t {
    Malloc = 0,
    New = 1,
    NewArray = 2,
    Memalign = 3,
};

enum State : uint8_t { Available = 0, Allocated = 1, Quarantined = 2 };

typedef uint64_t PackedHeader;
// Update the 'Mask' constants to reflect changes in this structure.
struct UnpackedHeader {
    uint64_t ClassId : 8;
    uint8_t  State : 2;
    // Origin if State == Allocated, or WasZeroed otherwise.
    uint8_t  OriginOrWasZeroed : 2;
    uint64_t SizeOrUnusedBytes : 20;
    uint64_t Offset : 16;
    uint64_t Checksum : 16;
};

constexpr uint64_t ClassIdMask = (1UL << 8) - 1;
constexpr uint64_t StateMask = (1U << 2) - 1;
constexpr uint64_t OriginMask = (1U << 2) - 1;
constexpr uint64_t SizeOrUnusedBytesMask = (1UL << 20) - 1;
constexpr uint64_t OffsetMask = (1UL << 16) - 1;
constexpr uint64_t ChecksumMask = (1UL << 16) - 1;

} // namespace Chunk
} // namespace scudo

#endif  // LLVM_SCUDO_CHUNK_H_
