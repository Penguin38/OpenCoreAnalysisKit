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
#include "runtime/art_method.h"
#include "runtime/oat.h"
#include "runtime/oat/stack_map.h"
#include "base/globals.h"
#include "base/bit_memory_region.h"
#include "android.h"
#include <string>

namespace art {

uint32_t CodeInfo::kNumHeaders = 0;
uint32_t CodeInfo::kNumBitTables = 8;

uint32_t StackMap::kNumStackMaps = 6;
uint32_t StackMap::kColNumKind = 0;
uint32_t StackMap::kColNumPackedNativePc = 1;
uint32_t StackMap::kColNumDexPc = 2;
uint32_t StackMap::kColNumRegisterMaskIndex = 3;
uint32_t StackMap::kColNumStackMaskIndex = 4;
uint32_t StackMap::kColNumInlineInfoIndex = 5;
uint32_t StackMap::kColNumDexRegisterMaskIndex = 6;
uint32_t StackMap::kColNumDexRegisterMapIndex = 7;

uint32_t RegisterMask::kNumRegisterMasks = 2;
uint32_t RegisterMask::kColNumValue = 0;
uint32_t RegisterMask::kColNumShift = 1;

uint32_t StackMask::kNumStackMasks = 1;
uint32_t StackMask::kColNumMask = 0;

uint32_t InlineInfo::kNumInlineInfos = 6;
uint32_t InlineInfo::kColNumIsLast = 0;
uint32_t InlineInfo::kColNumDexPc = 1;
uint32_t InlineInfo::kColNumMethodInfoIndex = 2;
uint32_t InlineInfo::kColNumArtMethodHi = 3;
uint32_t InlineInfo::kColNumArtMethodLo = 4;
uint32_t InlineInfo::kColNumNumberOfDexRegisters = 5;

uint32_t MethodInfo::kNumMethodInfos = 1;
uint32_t MethodInfo::kColNumMethodIndex = 0;
uint32_t MethodInfo::kColNumDexFileIndexKind = 1;
uint32_t MethodInfo::kColNumDexFileIndex = 2;

uint32_t DexRegisterMask::kNumDexRegisterMasks = 1;
uint32_t DexRegisterMask::kColNumMask = 0;

uint32_t DexRegisterMap::kNumDexRegisterMaps = 1;
uint32_t DexRegisterMap::kColNumCatalogueIndex = 0;

uint32_t DexRegisterInfo::kNumDexRegisterInfos = 2;
uint32_t DexRegisterInfo::kColNumKind = 0;
uint32_t DexRegisterInfo::kColNumPackedValue = 1;

void CodeInfo::OatInit124() {
    kNumHeaders = 2;
    kNumBitTables = 8;
}

void CodeInfo::OatInit150() {
    kNumBitTables = 8;
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

void StackMap::OatInit150() {
    kNumStackMaps = 6;
}

void StackMap::OatInit170() {
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

void RegisterMask::OatInit150() {
    kNumRegisterMasks = 2;
}

void RegisterMask::OatInit170() {
    kNumRegisterMasks = 2;
    kColNumValue = 0;
    kColNumShift = 1;
}

void StackMask::OatInit150() {
    kNumStackMasks = 1;
}

void StackMask::OatInit170() {
    kNumStackMasks = 1;
    kColNumMask = 0;
}

void InlineInfo::OatInit150() {
    kNumInlineInfos = 6;
}

void InlineInfo::OatInit170() {
    kNumInlineInfos = 6;
    kColNumIsLast = 0;
    kColNumDexPc = 1;
    kColNumMethodInfoIndex = 2;
    kColNumArtMethodHi = 3;
    kColNumArtMethodLo = 4;
    kColNumNumberOfDexRegisters = 5;
}

void MethodInfo::OatInit150() {
    kNumMethodInfos = 1;
}

void MethodInfo::OatInit170() {
    kNumMethodInfos = 1;
    kColNumMethodIndex = 0;
}

void MethodInfo::OatInit225() {
    kNumMethodInfos = 3;
    kColNumMethodIndex = 0;
    kColNumDexFileIndexKind = 1;
    kColNumDexFileIndex = 2;
}

void DexRegisterMask::OatInit150() {
    kNumDexRegisterMasks = 1;
}

void DexRegisterMask::OatInit170() {
    kNumDexRegisterMasks = 1;
    kColNumMask = 0;
}

void DexRegisterMap::OatInit150() {
    kNumDexRegisterMaps = 1;
}

void DexRegisterMap::OatInit170() {
    kNumDexRegisterMaps = 1;
    kColNumCatalogueIndex = 0;
}

void DexRegisterInfo::OatInit150() {
    kNumDexRegisterInfos = 2;
}

void DexRegisterInfo::OatInit170() {
    kNumDexRegisterInfos = 2;
    kColNumKind = 0;
    kColNumPackedValue = 1;
}

CodeInfo::CodeInfo(uint64_t code_info_data) {
    data_ = code_info_data;
    if (OatHeader::OatVersion() >= 150) {
        reader = code_info_data;
    }
}

void StackMapEncoding::Decode(const uint8_t** ptr) {
    dex_pc_bit_offset_ = (*ptr)[0];
    dex_register_map_bit_offset_ = (*ptr)[1];
    inline_info_bit_offset_ = (*ptr)[2];
    register_mask_index_bit_offset_ = (*ptr)[3];
    stack_mask_index_bit_offset_ = (*ptr)[4];
    total_bit_size_ = (*ptr)[5];
    (*ptr) += sizeof(StackMapEncoding);
}

void InvokeInfoEncoding::Decode(const uint8_t** ptr) {
    invoke_type_bit_offset_ = (*ptr)[0];
    method_index_bit_offset_ = (*ptr)[1];
    total_bit_size_ = (*ptr)[2];
    (*ptr) += sizeof(InvokeInfoEncoding);
}

void InlineInfoEncoding::Decode(const uint8_t** ptr) {
    dex_pc_bit_offset_ = (*ptr)[0];
    extra_data_bit_offset_ = (*ptr)[1];
    dex_register_map_bit_offset_ = (*ptr)[2];
    total_bit_size_ = (*ptr)[3];
    (*ptr) += sizeof(InlineInfoEncoding);
}

CodeInfoEncoding::CodeInfoEncoding(uint64_t code_info_data) {
    api::MemoryRef data = code_info_data;
    const uint8_t* ptr = reinterpret_cast<const uint8_t *>(data.Real());
    dex_register_map.Decode(&ptr);
    location_catalog.Decode(&ptr);
    stack_map.Decode(&ptr);
    register_mask.Decode(&ptr);
    stack_mask.Decode(&ptr);
    invoke_info.Decode(&ptr);
    if (stack_map.encoding.GetInlineInfoEncoding().BitSize() > 0) {
        inline_info.Decode(&ptr);
    } else {
        inline_info = BitEncodingTable<InlineInfoEncoding>();
    }
    cache_header_size = static_cast<uint32_t>(ptr - reinterpret_cast<const uint8_t *>(data.Real()));
    ComputeTableOffsets();
}

uint32_t CodeInfo::DecodeCodeSize(uint64_t code_info_data) {
    return DecodeHeaderOnly(code_info_data).code_size_;
}

QuickMethodFrameInfo CodeInfo::DecodeFrameInfo(uint64_t code_info_data) {
    CodeInfo code_info = DecodeHeaderOnly(code_info_data);
    return QuickMethodFrameInfo(code_info.frame_size_in_bytes_,
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
        code_info.frame_size_in_bytes_ = code_info.packed_frame_size_ * kStackAlignment;
    } else if (OatHeader::OatVersion() >= 173) {
        reader.ReadInterleavedVarints(kNumHeaders, header);
        code_info.flags_ = header[0];
        code_info.packed_frame_size_ = header[1];
        code_info.core_spill_mask_ = header[2];
        code_info.fp_spill_mask_ = header[3];
        code_info.number_of_dex_registers_ = header[4];
        code_info.bit_table_flags_ = header[5];
        code_info.frame_size_in_bytes_ = code_info.packed_frame_size_ * kStackAlignment;
    } else if (OatHeader::OatVersion() >= 172) {
        code_info.flags_ = reader.ReadVarint();
        code_info.packed_frame_size_ = reader.ReadVarint();
        code_info.core_spill_mask_ = reader.ReadVarint();
        code_info.fp_spill_mask_ = reader.ReadVarint();
        code_info.number_of_dex_registers_ = reader.ReadVarint();
        code_info.bit_table_flags_ = reader.ReadVarint();
        code_info.frame_size_in_bytes_ = code_info.packed_frame_size_ * kStackAlignment;
    } else if (OatHeader::OatVersion() >= 171) {
        code_info.flags_ = reader.ReadVarint();
        code_info.packed_frame_size_ = reader.ReadVarint();
        code_info.core_spill_mask_ = reader.ReadVarint();
        code_info.fp_spill_mask_ = reader.ReadVarint();
        code_info.number_of_dex_registers_ = reader.ReadVarint();
        code_info.frame_size_in_bytes_ = code_info.packed_frame_size_ * kStackAlignment;
    } else if (OatHeader::OatVersion() >= 159) {
        code_info.packed_frame_size_ = reader.ReadVarint();
        code_info.core_spill_mask_ = reader.ReadVarint();
        code_info.fp_spill_mask_ = reader.ReadVarint();
        code_info.number_of_dex_registers_ = reader.ReadVarint();
        code_info.frame_size_in_bytes_ = code_info.packed_frame_size_ * kStackAlignment;
    } else if (OatHeader::OatVersion() >= 151) {
        api::MemoryRef data = code_info_data;
        const uint8_t* ptr = reinterpret_cast<const uint8_t *>(data.Real());
        code_info.frame_size_in_bytes_ = DecodeUnsignedLeb128(&ptr);
        code_info.core_spill_mask_ = DecodeUnsignedLeb128(&ptr);
        code_info.fp_spill_mask_ = DecodeUnsignedLeb128(&ptr);
        code_info.number_of_dex_registers_ = DecodeUnsignedLeb128(&ptr);
        reader = BitMemoryReader(data.Ptr() + static_cast<uint32_t>(ptr - reinterpret_cast<const uint8_t *>(data.Real())));
    } else if (OatHeader::OatVersion() >= 150) {
        api::MemoryRef data = code_info_data;
        const uint8_t* ptr = reinterpret_cast<const uint8_t *>(data.Real());
        uint32_t non_header_size = DecodeUnsignedLeb128(&ptr);
        const uint8_t* end = ptr + non_header_size;
        code_info.frame_size_in_bytes_ = DecodeUnsignedLeb128(&ptr);
        code_info.core_spill_mask_ = DecodeUnsignedLeb128(&ptr);
        code_info.fp_spill_mask_ = DecodeUnsignedLeb128(&ptr);
        code_info.number_of_dex_registers_ = DecodeUnsignedLeb128(&ptr);
        code_info.region_ = MemoryRegion(data.Ptr() +
                                         static_cast<uint32_t>(ptr - reinterpret_cast<const uint8_t *>(data.Real())),
                                         end - ptr);
        reader = BitMemoryRegion(code_info.region_);
    } else if (OatHeader::OatVersion() >= 144) {
        api::MemoryRef data = code_info_data;
        const uint8_t* ptr = reinterpret_cast<const uint8_t *>(data.Real());
        uint32_t non_header_size = DecodeUnsignedLeb128(&ptr);
        code_info.region_ = MemoryRegion(data.Ptr() +
                                         static_cast<uint32_t>(ptr - reinterpret_cast<const uint8_t *>(data.Real())),
                                         non_header_size);
    }
    return code_info;
}

#define DECODE_BIT_TABLE_173(CODEINFO, READER, NUM, NAME) { \
    do { \
        if (CODEINFO.HasBitTable(NUM)) { \
            if (CODEINFO.IsBitTableDeduped(NUM)) { \
                int64_t bit_offset = READER.NumberOfReadBits() - READER.ReadVarint(); \
                BitMemoryReader reader2(READER.data(), bit_offset); \
                CODEINFO.Get##NAME().Decode(reader2); \
            } else { \
                uint64_t bit_offset = READER.NumberOfReadBits(); \
                CODEINFO.Get##NAME().Decode(READER); \
                READER.GetReadRegion().Subregion(bit_offset); \
            } \
        } \
    } while (0); \
}

#define DECODE_BIT_TABLE_172(CODEINFO, READER, NUM, NAME) { \
    do { \
        if (CODEINFO.HasBitTable(NUM)) { \
            if (CODEINFO.IsBitTableDeduped(NUM)) { \
                int64_t bit_offset = READER.NumberOfReadBits() - READER.ReadVarint(); \
                BitMemoryReader reader2(READER.data(), bit_offset); \
                CODEINFO.Get##NAME().Decode(reader2); \
            } else { \
                CODEINFO.Get##NAME().Decode(READER); \
            } \
        } \
    } while (0); \
}

#define DECODE_BIT_TABLE_159(CODEINFO, READER, NUM, NAME) { \
    do { \
        bool is_deduped = READER.ReadBit(); \
        if (is_deduped) { \
            int64_t bit_offset = READER.NumberOfReadBits() - READER.ReadVarint(); \
            BitMemoryReader reader2(READER.data(), bit_offset); \
            CODEINFO.Get##NAME().Decode(reader2); \
        } else { \
            CODEINFO.Get##NAME().Decode(reader); \
        } \
    } while (0); \
}

CodeInfo CodeInfo::Decode(uint64_t code_info_data) {
    CodeInfo code_info(code_info_data);
    if (OatHeader::OatVersion() >= 144) {
        code_info = DecodeHeaderOnly(code_info_data);
        BitMemoryReader& reader = code_info.GetMemoryReader();

        if (OatHeader::OatVersion() >= 173) {
            DECODE_BIT_TABLE_173(code_info, reader, 0, StackMap);
            DECODE_BIT_TABLE_173(code_info, reader, 1, RegisterMask);
            DECODE_BIT_TABLE_173(code_info, reader, 2, StackMask);
            DECODE_BIT_TABLE_173(code_info, reader, 3, InlineInfo);
            DECODE_BIT_TABLE_173(code_info, reader, 4, MethodInfo);
            DECODE_BIT_TABLE_173(code_info, reader, 5, DexRegisterMask);
            DECODE_BIT_TABLE_173(code_info, reader, 6, DexRegisterMap);
            DECODE_BIT_TABLE_173(code_info, reader, 7, DexRegisterInfo);
        } else if (OatHeader::OatVersion() >= 172) {
            DECODE_BIT_TABLE_172(code_info, reader, 0, StackMap);
            DECODE_BIT_TABLE_172(code_info, reader, 1, RegisterMask);
            DECODE_BIT_TABLE_172(code_info, reader, 2, StackMask);
            DECODE_BIT_TABLE_172(code_info, reader, 3, InlineInfo);
            DECODE_BIT_TABLE_172(code_info, reader, 4, MethodInfo);
            DECODE_BIT_TABLE_172(code_info, reader, 5, DexRegisterMask);
            DECODE_BIT_TABLE_172(code_info, reader, 6, DexRegisterMap);
            DECODE_BIT_TABLE_172(code_info, reader, 7, DexRegisterInfo);
        } else if (OatHeader::OatVersion() >= 159) {
            DECODE_BIT_TABLE_159(code_info, reader, 0, StackMap);
            DECODE_BIT_TABLE_159(code_info, reader, 1, RegisterMask);
            DECODE_BIT_TABLE_159(code_info, reader, 2, StackMask);
            DECODE_BIT_TABLE_159(code_info, reader, 3, InlineInfo);
            DECODE_BIT_TABLE_159(code_info, reader, 4, MethodInfo);
            DECODE_BIT_TABLE_159(code_info, reader, 5, DexRegisterMask);
            DECODE_BIT_TABLE_159(code_info, reader, 6, DexRegisterMap);
            DECODE_BIT_TABLE_159(code_info, reader, 7, DexRegisterInfo);
        }
    } else {
        code_info.encoding_ = CodeInfoEncoding(code_info_data);
        code_info.region_ = MemoryRegion(code_info_data,
                                         code_info.encoding_.HeaderSize()
                                         + code_info.encoding_.NonHeaderSize());
        code_info.number_of_stack_maps_ = code_info.encoding_.GetStackMap().NumEntries();
    }
    return code_info;
}

void CodeInfo::ExtendNumRegister(ArtMethod& method) {
    if (OatHeader::OatVersion() < 150) {
        art::dex::CodeItem item = method.GetCodeItem();
        art::DexFile& dex_file = method.GetDexFile();
        if (item.Ptr()) number_of_dex_registers_ = item.num_regs_;
    }
}

void StackMap::Decode(BitMemoryReader& reader) {
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
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void RegisterMask::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        value = header[1];
        shift = header[2];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void StackMask::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        mask = header[1];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void InlineInfo::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        is_last = header[1];
        dex_pc = header[2];
        method_info_index = header[3];
        art_method_hi = header[4];
        art_method_lo = header[5];
        number_of_dex_registers = header[6];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void MethodInfo::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 225) {
        method_index = header[1];
        dex_file_index_kind = header[2];
        dex_file_index = header[3];
    } else if (OatHeader::OatVersion() >= 170) {
        method_index = header[1];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void DexRegisterMask::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        mask = header[1];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void DexRegisterMap::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        catalogue_index = header[1];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void DexRegisterInfo::Decode(BitMemoryReader& reader) {
    std::vector<uint32_t> header;
    DecodeOnly(reader, header);
    if (OatHeader::OatVersion() >= 170) {
        kind = header[1];
        packed_value = header[2];
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

uint32_t StackMap::UnpackNativePc(uint32_t packed_native_pc) {
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
    uint32_t dex_pc = 0x0;
    if (OatHeader::OatVersion() >= 144) {
        StackMap& map = GetStackMap();
        if (!map.IsValid()) {
            return 0;
        }

        for (int row = 0; row < map.NumRows(); row++) {
            uint32_t packed_native_pc = map.Get(row, StackMap::kColNumPackedNativePc);
            uint32_t current_native_pc = StackMap::UnpackNativePc(packed_native_pc);
            if (current_native_pc > native_pc)
                break;
            dex_pc = map.Get(row, StackMap::kColNumDexPc);
        }
    } else {
        for (int row = 0; row < number_of_stack_maps_; row++) {
            BitMemoryRegion bit_region = encoding_.GetStackMap().BitRegion(region_, row);
            uint32_t current_native_pc = encoding_.GetStackMap().encoding.GetNativePcEncoding().Load(bit_region);
            if (current_native_pc > native_pc)
                break;
            dex_pc = encoding_.GetStackMap().encoding.GetDexPcEncoding().Load(bit_region);
        }

    }
    return dex_pc;
}

void CodeInfo::NativePc2VRegs(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vreg_map) {
    if (OatHeader::OatVersion() >= 170) {
        NativePc2VRegsV2(native_pc, vreg_map);
    } else {
        if (OatHeader::OatVersion() >= 144) {
            // do nothing
        } else {
            NativePc2VRegsV1(native_pc, vreg_map);
        }
    }
}

void CodeInfo::NativePc2VRegsV1(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vreg_map) {
    int32_t dex_register_map = -1;
    for (int row = 0; row < number_of_stack_maps_; row++) {
        BitMemoryRegion bit_region = encoding_.GetStackMap().BitRegion(region_, row);
        uint32_t current_native_pc = encoding_.GetStackMap().encoding.GetNativePcEncoding().Load(bit_region);
        if (current_native_pc > native_pc)
            break;
        dex_register_map = encoding_.GetStackMap().encoding.GetDexRegisterMapEncoding().Load(bit_region);
    }
    // TODO
}

void CodeInfo::NativePc2VRegsV2(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vreg_map) {
    StackMap& map = GetStackMap();
    if (!map.IsValid()) return;

    uint32_t current_row = BitTable::kNoValue;
    uint32_t dex_register_map_index = BitTable::kNoValue;
    for (int row = 0; row < map.NumRows(); row++) {
        uint32_t packed_native_pc = map.Get(row, StackMap::kColNumPackedNativePc);
        uint32_t current_native_pc = StackMap::UnpackNativePc(packed_native_pc);
        if (current_native_pc > native_pc)
            break;
        current_row = row;
        dex_register_map_index = map.Get(row, StackMap::kColNumDexRegisterMapIndex);
    }

    if (dex_register_map_index == BitTable::kNoValue) return;
    DexRegisterMap& dex_map = GetDexRegisterMap();
    if (!dex_map.IsValid()) return;

    DexRegisterMask& dex_mask = GetDexRegisterMask();
    if (!dex_mask.IsValid()) return;

    DexRegisterInfo& dex_info = GetDexRegisterInfo();
    if (!dex_info.IsValid()) return;

    std::map<uint32_t, uint32_t> bits_set;
    uint32_t remaining_registers = number_of_dex_registers_;
    for (int row = current_row; row >= 0 && remaining_registers != 0; row--) {
        uint32_t dex_register_mask_index = map.Get(row, StackMap::kColNumDexRegisterMaskIndex);
        if (dex_register_mask_index == BitTable::kNoValue)
            continue;

        if (dex_register_mask_index < dex_mask.NumRows()) {
            BitMemoryRegion mask = dex_mask.GetBitMemoryRegion(dex_register_mask_index, DexRegisterMask::kColNumMask);
            if (mask.size_in_bits() <= 0)
                continue;

            uint32_t map_index = map.Get(row, StackMap::kColNumDexRegisterMapIndex);
            map_index += mask.PopCount(0, 0);
            mask = mask.Subregion(0, mask.size_in_bits() - 0);
            uint32_t end = std::min<uint32_t>(number_of_dex_registers_, mask.size_in_bits());
            uint32_t kNumBits = std::numeric_limits<std::make_unsigned_t<uint32_t>>::digits;

            for (uint32_t reg = 0; reg < end; reg += kNumBits) {
                uint32_t bits = mask.LoadBits(reg, std::min<uint32_t>(end - reg, kNumBits));
                while (bits != 0) {
                    uint32_t bit = __builtin_ctz(bits);
                    uint32_t catalogue_index = dex_map.Get(map_index, DexRegisterMap::kColNumCatalogueIndex);
                    if (catalogue_index == BitTable::kNoValue) {
                        map_index++;
                        bits ^= 1u << bit;
                        continue;
                    }
                    uint32_t kind = dex_info.Get(catalogue_index, DexRegisterInfo::kColNumKind);
                    uint32_t value = dex_info.Get(catalogue_index, DexRegisterInfo::kColNumPackedValue);

                    if (!bits_set[reg + bit]) {
                        DexRegisterInfo info(kind, value);
                        vreg_map[reg + bit] = info;
                        remaining_registers--;
                        bits_set[reg + bit] = 1;
                    }
                    map_index++;
                    bits ^= 1u << bit;
                }
            }
        }
    }
}

std::string DexRegisterInfo::ConvertKindBit(DexRegisterInfo::KindBit kind) {
    switch(kind) {
        case DexRegisterInfo::KindBit::kInStack: return "stack";
        case DexRegisterInfo::KindBit::kInRegister: return "register";
        case DexRegisterInfo::KindBit::kInRegisterHigh: return "register high";
        case DexRegisterInfo::KindBit::kInFpuRegister: return "fpu register";
        case DexRegisterInfo::KindBit::kInFpuRegisterHigh: return "fpu register high";
        case DexRegisterInfo::KindBit::kConstant: return "constant";
        case DexRegisterInfo::KindBit::kInStackLargeOffset: return "stack (large offset)";
        case DexRegisterInfo::KindBit::kConstantLargeValue: return "constant (large value)";
    }
    return "";
}

void CodeInfo::Dump(const char* prefix) {
    std::string sub_prefix(prefix);
    sub_prefix.append("  ");
    if (OatHeader::OatVersion() >= 191) {
        LOGI("%sCodeInfo BitSize=%ld CodeSize:0x%x FrameSize:0x%x CoreSpillMask:0x%x FpSpillMask:0x%x NumberOfDexRegisters:%d\n",
                prefix, DataBitSize(), code_size_, packed_frame_size_ * kStackAlignment, core_spill_mask_, fp_spill_mask_, number_of_dex_registers_);
        GetStackMap().Dump(sub_prefix.c_str());
        GetRegisterMask().Dump(sub_prefix.c_str());
        GetStackMask().Dump(sub_prefix.c_str());
        GetInlineInfo().Dump(sub_prefix.c_str());
        GetMethodInfo().Dump(sub_prefix.c_str());
        GetDexRegisterMask().Dump(sub_prefix.c_str());
        GetDexRegisterMap().Dump(sub_prefix.c_str());
        GetDexRegisterInfo().Dump(sub_prefix.c_str());
    } else if (OatHeader::OatVersion() >= 150) {
        LOGI("%sCodeInfo BitSize=%ld FrameSize:0x%x CoreSpillMask:0x%x FpSpillMask:0x%x NumberOfDexRegisters:%d\n",
                prefix, DataBitSize(), packed_frame_size_ * kStackAlignment, core_spill_mask_, fp_spill_mask_, number_of_dex_registers_);
        GetStackMap().Dump(sub_prefix.c_str());
        GetRegisterMask().Dump(sub_prefix.c_str());
        GetStackMask().Dump(sub_prefix.c_str());
        GetInlineInfo().Dump(sub_prefix.c_str());
        GetMethodInfo().Dump(sub_prefix.c_str());
        GetDexRegisterMask().Dump(sub_prefix.c_str());
        GetDexRegisterMap().Dump(sub_prefix.c_str());
        GetDexRegisterInfo().Dump(sub_prefix.c_str());
    } else if (OatHeader::OatVersion() >= 124) {
        LOGI("%sCodeInfo (number_of_dex_registers=%d, number_of_stack_maps=%d)\n", prefix, number_of_dex_registers_, number_of_stack_maps_);
        GetEncoding().Dump(sub_prefix.c_str());
        std::string sub2_prefix(sub_prefix);
        sub2_prefix.append("  ");
        MemoryRegion location = region_.Subregion(GetEncoding().GetLocationCatalog().ByteOffset(), GetEncoding().GetLocationCatalog().NumBytes());
        uint32_t num = GetEncoding().GetLocationCatalog().NumEntryes();
        api::MemoryRef ref = location.pointer();
        for (int i = 0; i < num; ++i) {
            uint8_t value = ref.value8Of(i);
            DexRegisterInfo::KindBit kind = static_cast<DexRegisterInfo::KindBit>(value & 0x7);
            value = (value & 0xF8) >> 3;
            LOGI("%sentry %d in %s (%d)\n", sub2_prefix.c_str(), i,
                                            DexRegisterInfo::ConvertKindBit(kind).c_str(),
                                            value);
            // TODO
        }
    }
}

void StackMap::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sStackMap BitSize=%ld Rows=%d Bits={Kind=%d PackedNativePc=0x%x DexPc=0x%x RegisterMaskIndex=%d StackMaskIndex=%d InlineInfoIndex=%d DexRegisterMaskIndex=%d DexRegisterMapIndex=%d}\n",
                prefix, DataBitSize(), NumRows(), kind, packed_native_pc, dex_pc, register_mask_index, stack_mask_index, inline_info_index, dex_register_mask_index, dex_register_map_index);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void RegisterMask::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sRegisterMask BitSize=%ld Rows=%d Bits={Value=%d Shift=%d}\n", prefix, DataBitSize(), NumRows(), value, shift);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void StackMask::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sStackMask BitSize=%ld Rows=%d Bits={Mask=%d}\n", prefix, DataBitSize(), NumRows(), mask);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void InlineInfo::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sInlineInfo BitSize=%ld Rows=%d Bits={IsLast=%d DexPc=%d MethodInfoIndex=%d ArtMethodHi=%d ArtMethodLo=%d NumberOfDexRegisters=%d}\n",
                prefix, DataBitSize(), NumRows(), is_last, dex_pc, method_info_index, art_method_hi, art_method_lo, number_of_dex_registers);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void MethodInfo::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 225) {
        LOGI("%sMethodInfo BitSize=%ld Rows=%d Bits={MethodIndex=%d DexFileIndexKind=%d DexFileIndex=%d}\n",
                prefix, DataBitSize(), NumRows(), method_index, dex_file_index_kind, dex_file_index);
    } else if (OatHeader::OatVersion() >= 170) {
        LOGI("%sMethodInfo BitSize=%ld Rows=%d Bits={MethodIndex=%d}\n", prefix, DataBitSize(), NumRows(), method_index);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void DexRegisterMask::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sDexRegisterMask BitSize=%ld Rows=%d Bits={Mask=%d}\n", prefix, DataBitSize(), NumRows(), mask);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void DexRegisterMap::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sDexRegisterMap BitSize=%ld Rows=%d Bits={CatalogueIndex=%d}\n", prefix, DataBitSize(), NumRows(), catalogue_index);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void DexRegisterInfo::Dump(const char* prefix) {
    if (OatHeader::OatVersion() >= 170) {
        LOGI("%sDexRegisterInfo BitSize=%ld Rows=%d Bits={Kind=%d PackedValue=%d}\n", prefix, DataBitSize(), NumRows(), kind, packed_value);
    } else if (OatHeader::OatVersion() >= 150) {

    }
}

void CodeInfoEncoding::Dump(const char* prefix) {
    dex_register_map.Dump(prefix, "DexRegisterMap");
    stack_map.Dump(prefix);
    register_mask.Dump(prefix, "RegisterMask");
    stack_mask.Dump(prefix, "StackMask");
    invoke_info.Dump(prefix);
    inline_info.Dump(prefix);
    location_catalog.Dump(prefix, "DexRegisterLocationCatalog");
}

void StackMapEncoding::Dump(const char* prefix) {
    LOGI("%sStackMapEncoding (native_pc_bit_offset=%d, dex_pc_bit_offset=%d, dex_register_map_bit_offset=%d, inline_info_bit_offset=%d, register_mask_bit_offset=%d, stack_mask_index_bit_offset=%d, total_bit_size=%d)\n",
            prefix, kNativePcBitOffset, dex_pc_bit_offset_, dex_register_map_bit_offset_, inline_info_bit_offset_, register_mask_index_bit_offset_, stack_mask_index_bit_offset_, total_bit_size_);
}

void InvokeInfoEncoding::Dump(const char* prefix) {
    LOGI("%sInvokeInfoEncoding (native_pc_bit_offset=%d, invoke_type_bit_offset_=%d, method_index_bit_offset_=%d, total_bit_size_=%d)\n",
            prefix, kNativePcBitOffset, invoke_type_bit_offset_, method_index_bit_offset_, total_bit_size_);
}

void InlineInfoEncoding::Dump(const char* prefix) {
    LOGI("%sInlineInfoEncoding (method_index_bit_offset=%d, dex_pc_bit_offset=%d, extra_data_bit_offset=%d, dex_register_map_bit_offset=%d, total_bit_size=%d)\n",
            prefix, kMethodIndexBitOffset, dex_pc_bit_offset_, extra_data_bit_offset_, dex_register_map_bit_offset_, total_bit_size_);
}

void ByteSizedTable::Dump(const char* prefix, const char* name) {
    LOGI("%s%s (number_of_entries=%d, size_in_bytes=%d)\n", prefix, name, num_entries, num_bytes);
}

void BitRegionEncoding::Dump(const char* prefix, const char* name) {
    LOGI("%s%s (number_of_bits=%d)\n", prefix, name, num_bits);

}

} // namespace art
