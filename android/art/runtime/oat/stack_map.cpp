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

#include "runtime/oat/stack_map.h"
#include "base/globals.h"
#include "base/bit_memory_region.h"
#include "android.h"

namespace art {

uint32_t CodeInfo::kNumHeaders = 7;

void CodeInfo::Init() {
    kNumHeaders = 6;
}

void CodeInfo::Init31() {
    kNumHeaders = 7;
}

uint32_t CodeInfo::DecodeCodeSize(uint64_t code_info_data) {
    return DecodeHeaderOnly(code_info_data).code_size_;
}

QuickMethodFrameInfo CodeInfo::DecodeFrameInfo(uint64_t code_info_data) {
    CodeInfo code_info = DecodeHeaderOnly(code_info_data);
    return QuickMethodFrameInfo(code_info.packed_frame_size_ * kStackAlignment,
                                code_info.core_spill_mask_,
                                code_info.fp_spill_mask_);
}

CodeInfo CodeInfo::DecodeHeaderOnly(uint64_t code_info_data) {
    CodeInfo code_info;
    BitMemoryReader reader(code_info_data);
    std::vector<uint32_t> header;
    reader.ReadInterleavedVarints(kNumHeaders, header);
    if (Android::Sdk() >= Android::S) {
        code_info.flags_ = header[0];
        code_info.code_size_ = header[1];
        code_info.packed_frame_size_ = header[2];
        code_info.core_spill_mask_ = header[3];
        code_info.fp_spill_mask_ = header[4];
        code_info.number_of_dex_registers_ = header[5];
        code_info.bit_table_flags_ = header[6];
    } else {
        code_info.flags_ = header[0];
        code_info.packed_frame_size_ = header[1];
        code_info.core_spill_mask_ = header[2];
        code_info.fp_spill_mask_ = header[3];
        code_info.number_of_dex_registers_ = header[4];
        code_info.bit_table_flags_ = header[5];
    }
    return code_info;
}

} // namespace art
