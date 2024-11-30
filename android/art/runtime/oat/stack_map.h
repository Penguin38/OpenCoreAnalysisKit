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

#ifndef ANDROID_ART_RUNTIME_OAT_STACK_MAP_H_
#define ANDROID_ART_RUNTIME_OAT_STACK_MAP_H_

#include "runtime/quick/quick_method_frame_info.h"
#include "base/bit_memory_region.h"
#include "base/memory_region.h"
#include "base/leb128.h"
#include "base/bit_table.h"
#include <map>

namespace art {

class ArtMethod;

static constexpr uint32_t kFrameSlotSize = 4;

class GeneralStackMap {
public:
    uint32_t native_pc;
    uint32_t dex_pc;
};

class StackMap : public BitTable {
public:
    enum Kind {
        Default = -1,
        Catch = 0,
        OSR = 1,
        Debug = 2,
    };

    static void OatInit150();
    static void OatInit170();

    static uint32_t UnpackNativePc(uint32_t packed_native_pc);

    StackMap() : kind(0),
                 packed_native_pc(0),
                 dex_pc(0),
                 register_mask_index(0),
                 stack_mask_index(0),
                 inline_info_index(0),
                 dex_register_mask_index(0),
                 dex_register_map_index(0) {}
    uint32_t NumColumns() { return kNumStackMaps; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumStackMaps;
    static uint32_t kColNumKind;
    static uint32_t kColNumPackedNativePc;
    static uint32_t kColNumDexPc;
    static uint32_t kColNumRegisterMaskIndex;
    static uint32_t kColNumStackMaskIndex;
    static uint32_t kColNumInlineInfoIndex;
    static uint32_t kColNumDexRegisterMaskIndex;
    static uint32_t kColNumDexRegisterMapIndex;
private:
    uint32_t kind;
    uint32_t packed_native_pc;
    uint32_t dex_pc;
    uint32_t register_mask_index;
    uint32_t stack_mask_index;
    uint32_t inline_info_index;
    uint32_t dex_register_mask_index;
    uint32_t dex_register_map_index;
};

class RegisterMask : public BitTable {
public:
    static void OatInit150();
    static void OatInit170();

    RegisterMask() : value(0), shift(0) {}
    uint32_t NumColumns() { return kNumRegisterMasks; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumRegisterMasks;
    static uint32_t kColNumValue;
    static uint32_t kColNumShift;
private:
    uint32_t value;
    uint32_t shift;
};

class StackMask : public BitTable {
public:
    static void OatInit150();
    static void OatInit170();

    StackMask() : mask(0) {}
    uint32_t NumColumns() { return kNumStackMasks; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumStackMasks;
    static uint32_t kColNumMask;
private:
    uint32_t mask;
};

class InlineInfo : public BitTable {
public:
    static void OatInit150();
    static void OatInit170();

    InlineInfo() : is_last(0),
                   dex_pc(0),
                   method_info_index(0),
                   art_method_hi(0),
                   art_method_lo(0),
                   number_of_dex_registers(0) {}
    uint32_t NumColumns() { return kNumInlineInfos; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumInlineInfos;
    static uint32_t kColNumIsLast;
    static uint32_t kColNumDexPc;
    static uint32_t kColNumMethodInfoIndex;
    static uint32_t kColNumArtMethodHi;
    static uint32_t kColNumArtMethodLo;
    static uint32_t kColNumNumberOfDexRegisters;
private:
    uint32_t is_last;
    uint32_t dex_pc;
    uint32_t method_info_index;
    uint32_t art_method_hi;
    uint32_t art_method_lo;
    uint32_t number_of_dex_registers;
};

class MethodInfo : public BitTable {
public:
    static void OatInit150();
    static void OatInit170();
    static void OatInit225();

    MethodInfo() : method_index(0),
                   dex_file_index_kind(0),
                   dex_file_index(0) {}
    uint32_t NumColumns() { return kNumMethodInfos; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumMethodInfos;
    static uint32_t kColNumMethodIndex;
    static uint32_t kColNumDexFileIndexKind;
    static uint32_t kColNumDexFileIndex;
private:
    uint32_t method_index;
    uint32_t dex_file_index_kind;
    uint32_t dex_file_index;
};

class DexRegisterMask : public BitTable {
public:
    static void OatInit150();
    static void OatInit170();

    DexRegisterMask() : mask(0) {}
    uint32_t NumColumns() { return kNumDexRegisterMasks; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumDexRegisterMasks;
    static uint32_t kColNumMask;
private:
    uint32_t mask;
};

class DexRegisterMap : public BitTable {
public:
    static void OatInit150();
    static void OatInit170();

    DexRegisterMap() : catalogue_index(0) {}
    uint32_t NumColumns() { return kNumDexRegisterMaps; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumDexRegisterMaps;
    static uint32_t kColNumCatalogueIndex;
private:
    uint32_t catalogue_index;
};

class DexRegisterInfo : public BitTable {
public:

    enum class Kind : int32_t {
        kInvalid = -2,       // only used internally during register map decoding.
        kNone = -1,          // vreg has not been set.
        kInStack,            // vreg is on the stack, value holds the stack offset.
        kConstant,           // vreg is a constant value.
        kInRegister,         // vreg is in low 32 bits of a core physical register.
        kInRegisterHigh,     // vreg is in high 32 bits of a core physical register.
        kInFpuRegister,      // vreg is in low 32 bits of an FPU register.
        kInFpuRegisterHigh,  // vreg is in high 32 bits of an FPU register.
    };

    enum class KindBit : uint8_t {
        kInStack = 0,             // 0b000
        kInRegister = 1,          // 0b001
        kInRegisterHigh = 2,      // 0b010
        kInFpuRegister = 3,       // 0b011
        kInFpuRegisterHigh = 4,   // 0b100
        kConstant = 5,            // 0b101
        kInStackLargeOffset = 6,  // 0b110
        kConstantLargeValue = 7,  // 0b111
    };

    DexRegisterInfo()
        : kind(-2), packed_value(0) {}
    DexRegisterInfo(uint32_t k, uint32_t v)
        : kind(k), packed_value(v) {}
    DexRegisterInfo(Kind k, uint32_t v)
        : kind(static_cast<uint32_t>(k)), packed_value(v) {}

    static void OatInit150();
    static void OatInit170();

    uint32_t NumColumns() { return kNumDexRegisterInfos; }
    void Decode(BitMemoryReader& reader);
    void Dump(const char* prefix);

    static uint32_t kNumDexRegisterInfos;
    static uint32_t kColNumKind;
    static uint32_t kColNumPackedValue;

    inline uint32_t Kind() const { return kind; }
    inline uint32_t PackedValue() const { return packed_value; }

    static std::string ConvertKindBit(KindBit kind);
private:
    uint32_t kind;
    uint32_t packed_value;
};

class ByteSizedTable {
public:
    static constexpr uint32_t kInvalidOffset = static_cast<uint32_t>(-1);

    inline void Decode(const uint8_t** ptr) {
        num_entries = DecodeUnsignedLeb128(ptr);
        num_bytes = DecodeUnsignedLeb128(ptr);
    }

    void UpdateBitOffset(uint32_t* offset) {
        byte_offset = *offset / kBitsPerByte;
        *offset += num_bytes * kBitsPerByte;
    }

    inline uint32_t NumEntryes() { return num_entries; }
    inline uint32_t NumBytes() { return num_bytes; }
    inline uint32_t ByteOffset() { return byte_offset; }

    void Dump(const char* prefix, const char* name);
private:
    uint32_t num_entries = 0;
    uint32_t num_bytes = 0;
    uint32_t byte_offset = kInvalidOffset;
};

class FieldEncoding {
public:
    FieldEncoding(uint32_t start_offset, uint32_t end_offset, int32_t min_value = 0)
        : start_offset_(start_offset), end_offset_(end_offset), min_value_(min_value) {}

    inline uint32_t BitSize() { return end_offset_ - start_offset_; }
    inline int32_t Load(BitMemoryRegion region) {
        return static_cast<int32_t>(region.LoadBits(start_offset_, BitSize())) + min_value_;
    }
private:
    uint32_t start_offset_;
    uint32_t end_offset_;
    int32_t min_value_;
};

class StackMapEncoding {
public:
    StackMapEncoding()
        : dex_pc_bit_offset_(0),
        dex_register_map_bit_offset_(0),
        inline_info_bit_offset_(0),
        register_mask_index_bit_offset_(0),
        stack_mask_index_bit_offset_(0),
        total_bit_size_(0) {}

    void Decode(const uint8_t** ptr);
    inline uint32_t BitSize() { return total_bit_size_; }
    inline FieldEncoding GetNativePcEncoding() {
        return FieldEncoding(kNativePcBitOffset, dex_pc_bit_offset_, -1 /* min_value */);
    }
    inline FieldEncoding GetDexPcEncoding() {
        return FieldEncoding(dex_pc_bit_offset_, dex_register_map_bit_offset_, -1 /* min_value */);
    }
    inline FieldEncoding GetDexRegisterMapEncoding() {
        return FieldEncoding(dex_register_map_bit_offset_, inline_info_bit_offset_, -1 /* min_value */);
    }
    inline FieldEncoding GetInlineInfoEncoding() {
        return FieldEncoding(inline_info_bit_offset_, register_mask_index_bit_offset_, -1 /* min_value */);
    }

    void Dump(const char* prefix);
private:
    static constexpr uint32_t kNativePcBitOffset = 0;
    uint8_t dex_pc_bit_offset_;
    uint8_t dex_register_map_bit_offset_;
    uint8_t inline_info_bit_offset_;
    uint8_t register_mask_index_bit_offset_;
    uint8_t stack_mask_index_bit_offset_;
    uint8_t total_bit_size_;
};

class BitRegionEncoding {
public:
    inline void Decode(const uint8_t** ptr) {
        num_bits = DecodeUnsignedLeb128(ptr);
    }
    inline uint32_t BitSize() { return num_bits; }

    void Dump(const char* prefix, const char* name);
private:
    uint32_t num_bits = 0;
};

class InvokeInfoEncoding {
public:
    InvokeInfoEncoding()
        : invoke_type_bit_offset_(0),
        method_index_bit_offset_(0),
        total_bit_size_(0) {}

    inline uint32_t BitSize() { return total_bit_size_; }
    void Decode(const uint8_t** ptr);
    void Dump(const char* prefix);
private:
    static constexpr uint8_t kNativePcBitOffset = 0;
    uint8_t invoke_type_bit_offset_;
    uint8_t method_index_bit_offset_;
    uint8_t total_bit_size_;
};

class InlineInfoEncoding {
public:
    InlineInfoEncoding()
        : dex_pc_bit_offset_(0),
        extra_data_bit_offset_(0),
        dex_register_map_bit_offset_(0),
        total_bit_size_(0) {}

    inline uint32_t BitSize() { return total_bit_size_; }
    void Decode(const uint8_t** ptr);
    void Dump(const char* prefix);
private:
    static constexpr uint8_t kIsLastBitOffset = 0;
    static constexpr uint8_t kMethodIndexBitOffset = 1;
    uint8_t dex_pc_bit_offset_;
    uint8_t extra_data_bit_offset_;
    uint8_t dex_register_map_bit_offset_;
    uint8_t total_bit_size_;
};

template <typename Encoding>
class BitEncodingTable {
public:
    static constexpr uint32_t kInvalidOffset = static_cast<uint32_t>(-1);

    inline void Decode(const uint8_t** ptr) {
        num_entries = DecodeUnsignedLeb128(ptr);
        encoding.Decode(ptr);
    }

    void UpdateBitOffset(uint32_t* offset) {
        bit_offset = *offset;
        *offset += encoding.BitSize() * num_entries;
    }

    inline BitMemoryRegion BitRegion(MemoryRegion region, uint32_t index) {
        const uint32_t map_size = encoding.BitSize();
        return BitMemoryRegion(region, bit_offset + index * map_size, map_size);
    }

    inline void Dump(const char* prefix) { encoding.Dump(prefix); }
    inline void Dump(const char* prefix, const char* name) { encoding.Dump(prefix, name); }
    inline uint32_t NumEntries() { return num_entries; }
    Encoding encoding;
private:
    uint32_t num_entries = 0;
    uint32_t bit_offset = kInvalidOffset;
};

class CodeInfoEncoding {
public:
    static constexpr uint32_t kInvalidSize = std::numeric_limits<uint32_t>::max();

    CodeInfoEncoding() {}
    CodeInfoEncoding(uint64_t code_info_data);

    inline void ComputeTableOffsets() {
        uint32_t bit_offset = HeaderSize() * kBitsPerByte;
        dex_register_map.UpdateBitOffset(&bit_offset);
        location_catalog.UpdateBitOffset(&bit_offset);
        stack_map.UpdateBitOffset(&bit_offset);
        register_mask.UpdateBitOffset(&bit_offset);
        stack_mask.UpdateBitOffset(&bit_offset);
        invoke_info.UpdateBitOffset(&bit_offset);
        inline_info.UpdateBitOffset(&bit_offset);
        cache_non_header_size = RoundUp(bit_offset, kBitsPerByte) / kBitsPerByte - HeaderSize();
    }

    inline uint32_t HeaderSize() { return cache_header_size; }
    inline uint32_t NonHeaderSize() { return cache_non_header_size; }

    ByteSizedTable& GetDexRegisterMap() { return dex_register_map; }
    ByteSizedTable& GetLocationCatalog() { return location_catalog; }
    BitEncodingTable<StackMapEncoding>& GetStackMap() { return stack_map; }
    BitEncodingTable<BitRegionEncoding>& GetRegisterMask() { return register_mask; }
    BitEncodingTable<BitRegionEncoding>& GetStackMask() { return stack_mask; }
    BitEncodingTable<InvokeInfoEncoding>& GetInvokeInfo() { return invoke_info; }
    BitEncodingTable<InlineInfoEncoding>& GetInlineInfo() { return inline_info; }

    void Dump(const char* prefix);
private:
    uint32_t cache_header_size = kInvalidSize;
    uint32_t cache_non_header_size = kInvalidSize;

    ByteSizedTable dex_register_map;
    ByteSizedTable location_catalog;
    BitEncodingTable<StackMapEncoding> stack_map;
    BitEncodingTable<BitRegionEncoding> register_mask;
    BitEncodingTable<BitRegionEncoding> stack_mask;
    BitEncodingTable<InvokeInfoEncoding> invoke_info;
    BitEncodingTable<InlineInfoEncoding> inline_info;
};

class CodeInfo {
public:
    static CodeInfo Decode(uint64_t code_info_data);
    static CodeInfo DecodeHeaderOnly(uint64_t code_info_data);
    static uint32_t DecodeCodeSize(uint64_t code_info_data);
    static QuickMethodFrameInfo DecodeFrameInfo(uint64_t code_info_data);

    CodeInfo(uint64_t code_info_data);

    enum Flags {
        kHasInlineInfo = 1 << 0,
    };

    static void Init();
    static void OatInit124(); // 8.0.0_r1 Base
    static void OatInit150(); // Add method frame info to CodeInfo.
    static void OatInit171(); // Optimize stack maps: add fast path for no inline info.
    static void OatInit172(); // Stack maps: Handle special cases using flags.
    static void OatInit191(); // Add code size to CodeInfo.

    bool HasBitTable(uint32_t i ) { return ((bit_table_flags_ >> i) & 1) != 0; }
    bool IsBitTableDeduped(uint32_t i) { return ((bit_table_flags_ >> (kNumBitTables + i)) & 1) != 0; }
    bool HasDedupedBitTables() { return (bit_table_flags_ >> kNumBitTables) != 0u; }
    inline uint64_t DataBitSize() { return reader.NumberOfReadBits(); }

    BitMemoryReader& GetMemoryReader() { return reader; }
    StackMap& GetStackMap() { return stack_map_; }
    RegisterMask& GetRegisterMask() { return register_mask_; }
    StackMask& GetStackMask() { return stack_mask_; }
    InlineInfo& GetInlineInfo() { return inline_info_; }
    MethodInfo& GetMethodInfo() { return method_info_; }
    DexRegisterMask& GetDexRegisterMask() { return dex_register_mask_; }
    DexRegisterMap& GetDexRegisterMap() { return dex_register_map_; }
    DexRegisterInfo& GetDexRegisterInfo() { return dex_register_info_; }

    // 124+
    CodeInfoEncoding& GetEncoding() { return encoding_; }

    uint32_t NativePc2DexPc(uint32_t native_pc);
    void NativePc2VRegs(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vregs);
    void NativeStackMaps(std::vector<GeneralStackMap>& maps);
    void ExtendNumRegister(ArtMethod& method);

    void Dump(const char* prefix);

    static uint32_t kNumHeaders;
    static uint32_t kNumBitTables;

protected:
    MemoryRegion region_;
    CodeInfoEncoding encoding_;
private:
    void NativePc2VRegsV1(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vregs);
    void NativePc2VRegsV2(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vregs);

    uint64_t data_ = 0;
    BitMemoryReader reader = 0;
    uint32_t flags_ = 0;                    // 171+
    uint32_t code_size_ = 0;                // 191+, The size of native PC range in bytes.
    uint32_t packed_frame_size_ = 0;        // 150+, Frame size in kStackAlignment units.
    uint32_t core_spill_mask_ = 0;          // 150+
    uint32_t fp_spill_mask_ = 0;            // 150+
    uint32_t number_of_dex_registers_ = 0;  // 150+
    uint32_t bit_table_flags_ = 0;          // 172+

    // 124+
    uint32_t number_of_stack_maps_ = 0;
    uint32_t frame_size_in_bytes_ = 0;

    // Bit tables
    StackMap stack_map_;
    RegisterMask register_mask_;
    StackMask stack_mask_;
    InlineInfo inline_info_;
    MethodInfo method_info_;
    DexRegisterMask dex_register_mask_;
    DexRegisterMap dex_register_map_;
    DexRegisterInfo dex_register_info_;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_OAT_STACK_MAP_H_
