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
#include "base/bit_utils.h"
#include "base/bit_memory_region.h"

namespace art {

uint64_t BitMemoryRegion::LoadBits(uint64_t bit_offset, uint64_t bit_length) {
    if (bit_length == 0) {
        return 0;
    }
    uint32_t point_size = CoreApi::GetPointSize();
    uint64_t width = CoreApi::Bits();
    uint64_t index = (bit_start_ + bit_offset) / width;
    uint64_t shift = (bit_start_ + bit_offset) % width;
    uint64_t value = data_.valueOf(index * point_size) >> shift;
    uint64_t extra = data_.valueOf((index + (shift + (bit_length - 1)) / width) * point_size);
    uint64_t clear = (std::numeric_limits<uint64_t>::max() << 1) << (bit_length - 1);
    return (value | (extra << ((width - shift) & (width - 1)))) & ~clear;
}

uint64_t BitMemoryRegion::LoadBits64(uint64_t bit_offset, uint64_t bit_length) {
    if (bit_length == 0) {
        return 0;
    }
    uint32_t point_size = 8;
    uint64_t width = 64;
    uint64_t index = (bit_start_ + bit_offset) / width;
    uint64_t shift = (bit_start_ + bit_offset) % width;
    uint64_t value = data_.value64Of(index * point_size) >> shift;
    uint64_t extra = data_.value64Of((index + (shift + (bit_length - 1)) / width) * point_size);
    uint64_t clear = (std::numeric_limits<uint64_t>::max() << 1) << (bit_length - 1);
    return (value | (extra << ((width - shift) & (width - 1)))) & ~clear;
}

uint64_t BitMemoryRegion::PopCount(uint64_t bit_offset, uint64_t bit_length) {
    return Subregion(bit_offset, bit_length).PopCount();
}
uint64_t BitMemoryRegion::PopCount() {
    uint64_t result = 0u;
    VisitChunks([&](uint64_t /*offset*/,
                    uint64_t /*num_bits*/,
                    uint64_t value) {
                        result += POPCOUNT(value);
                        return true;
                    });
    return result;
}

template <typename VisitorType>
bool BitMemoryRegion::VisitChunks(VisitorType&& visitor) {
    uint64_t kChunkSize = CoreApi::Bits();
    uint64_t remaining_bits = bit_size_;
    if (remaining_bits == 0) {
        return true;
    }

    api::MemoryRef data = data_;
    uint32_t point_size = CoreApi::GetPointSize();
    uint64_t offset = 0u;
    uint64_t bit_start = bit_start_;
    data.MovePtr(bit_start / kChunkSize * point_size);
    if ((bit_start % kChunkSize) != 0u) {
        uint64_t leading_bits = kChunkSize - (bit_start % kChunkSize);
        uint64_t value = (data.valueOf()) >> (bit_start % kChunkSize);
        if (leading_bits > remaining_bits) {
            leading_bits = remaining_bits;
            value = value & ~(std::numeric_limits<uint64_t>::max() << remaining_bits);
        }
        if (!visitor(offset, leading_bits, value)) {
            return false;
        }
        offset += leading_bits;
        remaining_bits -= leading_bits;
        data.MovePtr(point_size);
    }
    while (remaining_bits >= kChunkSize) {
        uint64_t value = data.valueOf();
        if (!visitor(offset, kChunkSize, value)) {
            return false;
        }
        offset += kChunkSize;
        remaining_bits -= kChunkSize;
        data.MovePtr(point_size);
    }
    if (remaining_bits != 0u) {
        uint64_t value = (data.valueOf()) & ~(std::numeric_limits<uint64_t>::max() << remaining_bits);
        if (!visitor(offset, remaining_bits, value)) {
            return false;
        }
    }
    return true;
}

void BitMemoryReader::ReadInterleavedVarints(uint32_t num, std::vector<uint32_t>& values) {
    uint64_t data = ReadBits64(num * kVarintBits);
    for (uint32_t i = 0; i < num; i++) {
        values.push_back(BitFieldExtract(data, i * kVarintBits, kVarintBits));
    }

    for (uint32_t i = 0; i < num; i++) {
        if (values[i] > kVarintMax) {
            values[i] = ReadBits((values[i] - kVarintMax) * kBitsPerByte);
        }
    }
}

} // namespace art

