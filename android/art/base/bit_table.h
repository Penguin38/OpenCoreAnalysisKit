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

#ifndef ANDROID_ART_BASE_BIT_TABLE_H_
#define ANDROID_ART_BASE_BIT_TABLE_H_

#include "logger/log.h"
#include "base/bit_memory_region.h"
#include <numeric>

namespace art {

class BitTable {
public:
    static constexpr uint32_t kNoValue = std::numeric_limits<uint32_t>::max();
    static constexpr uint32_t kValueBias = kNoValue;

    BitTable() {}

    virtual ~BitTable() {};
    virtual uint32_t NumColumns() = 0;
    virtual void Decode(BitMemoryReader& reader) = 0;

    inline uint32_t NumRows() { return num_rows_; }
    inline uint32_t NumRowBits() { return column_offset_[NumColumns()]; }
    inline uint32_t NumColumnBits(uint32_t column) {
        return column_offset_[column + 1] - column_offset_[column];
    }
    inline uint64_t DataBitSize() { return table_data_.size_in_bits(); }

    inline uint32_t Get(uint32_t row, uint32_t column = 0) {
        uint64_t offset = row * NumRowBits() + column_offset_[column];
        return table_data_.LoadBits(offset, NumColumnBits(column)) + kValueBias;
    }

    inline BitMemoryRegion GetBitMemoryRegion(uint32_t row, uint32_t column = 0) {
        uint64_t offset = row * NumRowBits() + column_offset_[column];
        return table_data_.Subregion(offset, NumColumnBits(column));
    }

    inline bool IsValid() { return num_rows_ > 0; }
    inline void DecodeOnly(BitMemoryReader& reader, std::vector<uint32_t>& header) {
        uint32_t kNumColumns = NumColumns();
        reader.ReadInterleavedVarints(kNumColumns + 1, header);
        num_rows_ = header[0];
        column_offset_.resize(kNumColumns + 1, 0);
        column_offset_[0] = 0;
        for (uint32_t i = 0; i < kNumColumns; ++i) {
            uint32_t column_end = column_offset_[i] + header[i + 1];
            column_offset_[i + 1] = static_cast<uint16_t>(column_end);
        }
        table_data_ = reader.ReadRegion(num_rows_ * NumRowBits());
    }

private:
    BitMemoryRegion table_data_;
    uint32_t num_rows_ = 0;
    std::vector<uint16_t> column_offset_;
};

} // namespace art

#endif // ANDROID_ART_BASE_BIT_TABLE_H_
