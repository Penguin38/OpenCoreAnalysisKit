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
#include "runtime/oat.h"
#include "runtime/oat/stack_map.h"
#include "base/globals.h"
#include "base/bit_memory_region.h"
#include "android.h"

namespace art {

uint32_t CodeInfo::kNumHeaders = 0;
uint32_t CodeInfo::kNumBitTables = 8;
uint32_t CodeInfo::StackMap::kNumStackMaps = 6;

void CodeInfo::OatInit124() {
    kNumHeaders = 0;
    kNumBitTables = 8;
}

void CodeInfo::OatInit150() {
    kNumHeaders = 4;
}

void CodeInfo::OatInit171() {
    kNumHeaders = 5;
}

void CodeInfo::OatInit172() {
    kNumHeaders = 6;
}

void CodeInfo::OatInit191() {
    kNumHeaders = 7;
}

void CodeInfo::StackMap::OatInit124() {
    kNumStackMaps = 6;
}

void CodeInfo::StackMap::OatInit170() {
    kNumStackMaps = 8;
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
    CodeInfo code_info(code_info_data);
    BitMemoryReader& reader = code_info.GetMemoryReader();
    std::vector<uint32_t> header;
    if (OatHeader::OatVersion() >= 191) {
        reader.ReadInterleavedVarints(kNumHeaders, header);
        code_info.flags_ = header[0];
        code_info.code_size_ = header[1];
        code_info.packed_frame_size_ = header[2];
        code_info.core_spill_mask_ = header[3];
        code_info.fp_spill_mask_ = header[4];
        code_info.number_of_dex_registers_ = header[5];
        code_info.bit_table_flags_ = header[6];
    } else if (OatHeader::OatVersion() >= 172) {
        reader.ReadInterleavedVarints(kNumHeaders, header);
        code_info.flags_ = header[0];
        code_info.packed_frame_size_ = header[1];
        code_info.core_spill_mask_ = header[2];
        code_info.fp_spill_mask_ = header[3];
        code_info.number_of_dex_registers_ = header[4];
        code_info.bit_table_flags_ = header[5];
    } else if (OatHeader::OatVersion() >= 171) {
        reader.ReadInterleavedVarints(kNumHeaders, header);
        code_info.flags_ = header[0];
        code_info.packed_frame_size_ = header[1];
        code_info.core_spill_mask_ = header[2];
        code_info.fp_spill_mask_ = header[3];
        code_info.number_of_dex_registers_ = header[4];
    } else if (OatHeader::OatVersion() >= 150) {
        reader.ReadInterleavedVarints(kNumHeaders, header);
        code_info.packed_frame_size_ = header[0];
        code_info.core_spill_mask_ = header[1];
        code_info.fp_spill_mask_ = header[2];
        code_info.number_of_dex_registers_ = header[3];
    }
    return code_info;
}

CodeInfo CodeInfo::Decode(uint64_t code_info_data) {
    CodeInfo code_info = DecodeHeaderOnly(code_info_data);
    BitMemoryReader& reader = code_info.GetMemoryReader();

    if (code_info.HasBitTable(0)) {
        code_info.GetStackMap().Decode(reader);
    }

    return code_info;
}

void CodeInfo::StackMap::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        kind = header[1];
        packed_native_pc = header[2];
        dex_pc = header[3];
        register_mask_index = header[4];
        stack_mask_index = header[5];
        inline_info_index = header[6];
        dex_register_mask_index = header[7];
        dex_register_map_index = header[8];
    } else if (OatHeader::OatVersion() >= 124) {

    }
}

void CodeInfo::Dump(const char* prefix) {
    std::string sub_prefix(prefix);
    sub_prefix.append("  ");
    if (OatHeader::OatVersion() >= 191) {
        LOGI("%sCodeInfo CodeSize:0x%x FrameSize:0x%x CoreSpillMask:0x%x FpSpillMask:0x%x NumberOfDexRegisters:%d\n",
                prefix, code_size_, packed_frame_size_ * kStackAlignment, core_spill_mask_, fp_spill_mask_, number_of_dex_registers_);
        GetStackMap().Dump(sub_prefix.c_str());
    } else if (OatHeader::OatVersion() >= 150) {
        LOGI("%sCodeInfo FrameSize:0x%x CoreSpillMask:0x%x FpSpillMask:0x%x NumberOfDexRegisters:%d\n",
                prefix, packed_frame_size_ * kStackAlignment, core_spill_mask_, fp_spill_mask_, number_of_dex_registers_);
        GetStackMap().Dump(sub_prefix.c_str());
    }
}

void CodeInfo::StackMap::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sStackMap Rows=%d Bits={Kind=%d PackedNativePc=0x%x DexPc=0x%x RegisterMaskIndex=%d StackMaskIndex=%d InlineInfoIndex=%d DexRegisterMaskIndex=%d DexRegisterMapIndex=%d}\n",
                prefix, NumRows(), kind, packed_native_pc, dex_pc, register_mask_index, stack_mask_index, inline_info_index, dex_register_mask_index, dex_register_map_index);
    } else if (OatHeader::OatVersion() >= 124) {

    }
}

} // namespace art
