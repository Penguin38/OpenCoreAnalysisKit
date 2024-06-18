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

#include "logger/log.h"
#include "android.h"
#include "runtime/oat.h"
#include "base/bit_table.h"

namespace art {

void BitTable::DecodeOnly(BitMemoryReader& reader, std::vector<uint32_t>& header) {
    if (OatHeader::OatVersion() >= 171) {
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
    } else if (OatHeader::OatVersion() >= 170) {
        uint32_t kNumColumns = NumColumns();
        column_offset_.resize(kNumColumns + 1, 0);
        header.push_back(reader.ReadVarint());
        num_rows_ = header[0];
        if (num_rows_ != 0) {
            column_offset_[0] = 0;
            for (uint32_t i = 0; i < kNumColumns; ++i) {
                header.push_back(reader.ReadVarint());
                uint32_t column_end = column_offset_[i] + header[i + 1];
                column_offset_[i + 1] = static_cast<uint16_t>(column_end);
            }
        }
        table_data_ = reader.ReadRegion(num_rows_ * NumRowBits());
    }
}

} // namespace art
