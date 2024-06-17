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
#include "api/core.h"
#include "common/elf.h"
#include "runtime/oat.h"
#include "runtime/oat/stack_map.h"
#include "base/globals.h"
#include "base/bit_memory_region.h"
#include "android.h"

namespace art {

uint32_t CodeInfo::kNumHeaders = 0;
uint32_t CodeInfo::kNumBitTables = 8;

uint32_t CodeInfo::StackMap::kNumStackMaps = 6;
uint32_t CodeInfo::StackMap::kColNumKind = 0;
uint32_t CodeInfo::StackMap::kColNumPackedNativePc = 1;
uint32_t CodeInfo::StackMap::kColNumDexPc = 2;
uint32_t CodeInfo::StackMap::kColNumRegisterMaskIndex = 3;
uint32_t CodeInfo::StackMap::kColNumStackMaskIndex = 4;
uint32_t CodeInfo::StackMap::kColNumInlineInfoIndex = 5;
uint32_t CodeInfo::StackMap::kColNumDexRegisterMaskIndex = 6;
uint32_t CodeInfo::StackMap::kColNumDexRegisterMapIndex = 7;

uint32_t CodeInfo::RegisterMask::kNumRegisterMasks = 2;
uint32_t CodeInfo::RegisterMask::kColNumValue = 0;
uint32_t CodeInfo::RegisterMask::kColNumShift = 1;

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
    kColNumKind = 0;
    kColNumPackedNativePc = 1;
    kColNumDexPc = 2;
    kColNumRegisterMaskIndex = 3;
    kColNumStackMaskIndex = 4;
    kColNumInlineInfoIndex = 5;
    kColNumDexRegisterMaskIndex = 6;
    kColNumDexRegisterMapIndex = 7;
}

void CodeInfo::RegisterMask::OatInit124() {
    kNumRegisterMasks = 6;
}

void CodeInfo::RegisterMask::OatInit170() {
    kNumRegisterMasks = 2;
    kColNumValue = 0;
    kColNumShift = 1;
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

#define DECODE_BITTABLE(CODEINFO, READER, NUM, NAME) { \
    do { \
        if (CODEINFO.HasBitTable(NUM)) { \
            if (CODEINFO.IsBitTableDeduped(NUM)) { \
                uint64_t bit_offset = READER.NumberOfReadBits() - READER.ReadVarint(); \
                BitMemoryReader reader2(READER.data(), bit_offset); \
                code_info.Get##NAME().Decode(reader2); \
            } else { \
                uint64_t bit_offset = reader.NumberOfReadBits(); \
                code_info.Get##NAME().Decode(reader); \
                READER.GetReadRegion().Subregion(bit_offset); \
            } \
        } \
    } while (0); \
}

CodeInfo CodeInfo::Decode(uint64_t code_info_data) {
    CodeInfo code_info = DecodeHeaderOnly(code_info_data);
    BitMemoryReader& reader = code_info.GetMemoryReader();

    DECODE_BITTABLE(code_info, reader, 0, StackMap);
    DECODE_BITTABLE(code_info, reader, 1, RegisterMask);

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

void CodeInfo::RegisterMask::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        value = header[1];
        shift = header[2];
    } else if (OatHeader::OatVersion() >= 124) {

    }
}

uint32_t CodeInfo::StackMap::UnpackNativePc(uint32_t packed_native_pc) {
    int machine = CoreApi::GetMachine();
    switch (machine) {
        case EM_386: return packed_native_pc;
        case EM_X86_64: return packed_native_pc;
        case EM_ARM: return packed_native_pc * 2;
        case EM_AARCH64: return packed_native_pc * 4;
        case EM_RISCV: return packed_native_pc * 2;
    }
    return packed_native_pc;
}

uint32_t CodeInfo::NativePc2DexPc(uint32_t native_pc) {
    StackMap& map = GetStackMap();
    if (!map.IsValid()) {
        return 0;
    }

    uint32_t dex_pc = 0x0;
    for (int row = 0; row < map.NumRows(); row++) {
        uint32_t packed_native_pc = map.Get(row, CodeInfo::StackMap::kColNumPackedNativePc);
        uint32_t current_native_pc = CodeInfo::StackMap::UnpackNativePc(packed_native_pc);
        if (current_native_pc > native_pc)
            break;
        dex_pc = map.Get(row, CodeInfo::StackMap::kColNumDexPc);
    }
    return dex_pc;
}

void CodeInfo::Dump(const char* prefix) {
    std::string sub_prefix(prefix);
    sub_prefix.append("  ");
    if (OatHeader::OatVersion() >= 191) {
        LOGI("%sCodeInfo CodeSize:0x%x FrameSize:0x%x CoreSpillMask:0x%x FpSpillMask:0x%x NumberOfDexRegisters:%d\n",
                prefix, code_size_, packed_frame_size_ * kStackAlignment, core_spill_mask_, fp_spill_mask_, number_of_dex_registers_);
        GetStackMap().Dump(sub_prefix.c_str());
        GetRegisterMask().Dump(sub_prefix.c_str());
    } else if (OatHeader::OatVersion() >= 150) {
        LOGI("%sCodeInfo FrameSize:0x%x CoreSpillMask:0x%x FpSpillMask:0x%x NumberOfDexRegisters:%d\n",
                prefix, packed_frame_size_ * kStackAlignment, core_spill_mask_, fp_spill_mask_, number_of_dex_registers_);
        GetStackMap().Dump(sub_prefix.c_str());
        GetRegisterMask().Dump(sub_prefix.c_str());
    }
}

void CodeInfo::StackMap::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sStackMap Rows=%d Bits={Kind=%d PackedNativePc=0x%x DexPc=0x%x RegisterMaskIndex=%d StackMaskIndex=%d InlineInfoIndex=%d DexRegisterMaskIndex=%d DexRegisterMapIndex=%d}\n",
                prefix, NumRows(), kind, packed_native_pc, dex_pc, register_mask_index, stack_mask_index, inline_info_index, dex_register_mask_index, dex_register_map_index);
    } else if (OatHeader::OatVersion() >= 124) {

    }
}

void CodeInfo::RegisterMask::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sRegisterMask Rows=%d Bits={Value=%d Shift=%d}\n", prefix, NumRows(), value, shift);
    } else if (OatHeader::OatVersion() >= 124) {

    }
}

} // namespace art
