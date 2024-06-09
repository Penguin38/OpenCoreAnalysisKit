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

#ifndef ANDROID_ART_BASE_BIT_MEMORY_REGION_H_
#define ANDROID_ART_BASE_BIT_MEMORY_REGION_H_

#include "api/memory_ref.h"
#include "common/bit.h"
#include "base/globals.h"
#include "base/bit_utils.h"
#include <vector>
#include <array>

namespace art {

class BitMemoryRegion {
public:
    BitMemoryRegion() = default;
    BitMemoryRegion(uint64_t data, uint64_t bit_start, uint64_t bit_size) {
        data_ = RoundDown(data + (bit_start >> kBitsPerByteLog2), kPageSize);
        bit_start_ = bit_start + kBitsPerByte * (data - data_.Ptr());
        bit_size_ = bit_size;
    }
    uint64_t data() {
        return data_.Ptr() + bit_start_ / kBitsPerByte;
    }
    uint64_t size_in_bits() { return bit_size_; }
    void Resize(uint64_t bit_size) { bit_size_ = bit_size; }
    BitMemoryRegion Subregion(uint64_t bit_offset, uint64_t bit_length) {
        BitMemoryRegion result = *this;
        result.bit_start_ += bit_offset;
        result.bit_size_ = bit_length;
        return result;
    }
    BitMemoryRegion Subregion(uint64_t bit_offset) {
        BitMemoryRegion result = *this;
        result.bit_start_ += bit_offset;
        result.bit_size_ -= bit_offset;
        return result;
    }
    uint64_t LoadBits(uint64_t bit_offset, uint64_t bit_length);
    uint64_t PopCount(uint64_t bit_offset, uint64_t bit_length) {
        return Subregion(bit_offset, bit_length).PopCount();
    }
    uint64_t PopCount() {
        uint64_t result = 0u;
        VisitChunks([&](uint64_t /*offset*/,
                        uint64_t /*num_bits*/,
                        uint64_t value) {
                            result += POPCOUNT(value);
                            return true;
                        });
        return result;
    }
    template <typename VisitorType> bool VisitChunks(VisitorType&& visitor);
private:
    api::MemoryRef data_;
    uint64_t bit_start_;
    uint64_t bit_size_;
};

constexpr uint32_t kVarintBits = 4;
constexpr uint32_t kVarintMax = 11;

class BitMemoryReader {
public:
    BitMemoryReader(uint64_t data, uint64_t bit_offset = 0)
        : finished_region_(data, bit_offset, 0) {}
    BitMemoryRegion ReadRegion(uint64_t bit_length) {
        uint64_t bit_offset = finished_region_.size_in_bits();
        finished_region_.Resize(bit_offset + bit_length);
        return finished_region_.Subregion(bit_offset, bit_length);
    }
    uint64_t ReadBits(uint64_t bit_length) {
        return ReadRegion(bit_length).LoadBits(/* bit_offset */ 0, bit_length);
    }
    uint64_t data() { return finished_region_.data(); }
    BitMemoryRegion GetReadRegion() { return finished_region_; }
    uint64_t NumberOfReadBits() { return finished_region_.size_in_bits(); }
    uint32_t ReadVarint() {
        uint32_t x = ReadBits(kVarintBits);
        return (x <= kVarintMax) ? x : ReadBits((x - kVarintMax) * kBitsPerByte);
    }
    void ReadInterleavedVarints(uint32_t num, std::vector<uint32_t>& values);
private:
    BitMemoryRegion finished_region_;
};

} // namespace art

#endif  // ANDROID_ART_BASE_BIT_MEMORY_REGION_H_
