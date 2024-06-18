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
#include "base/bit_table.h"
#include <map>

namespace art {

static constexpr uint32_t kFrameSlotSize = 4;

class CodeInfo {
public:
    static CodeInfo Decode(uint64_t code_info_data);
    static CodeInfo DecodeHeaderOnly(uint64_t code_info_data);
    static uint32_t DecodeCodeSize(uint64_t code_info_data);
    static QuickMethodFrameInfo DecodeFrameInfo(uint64_t code_info_data);

    CodeInfo(uint64_t code_info_data) {
        reader = code_info_data;
    }

    enum Flags {
        kHasInlineInfo = 1 << 0,
    };

    class StackMap : public BitTable {
    public:
        enum Kind {
            Default = -1,
            Catch = 0,
            OSR = 1,
            Debug = 2,
        };

        static void OatInit124();
        static void OatInit170();

        static uint32_t UnpackNativePc(uint32_t packed_native_pc);

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
        static void OatInit124();
        static void OatInit170();

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
        static void OatInit124();
        static void OatInit170();

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
        static void OatInit124();
        static void OatInit170();

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
        static void OatInit124();
        static void OatInit170();
        static void OatInit225();

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
        static void OatInit124();
        static void OatInit170();

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
        static void OatInit124();
        static void OatInit170();

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

        DexRegisterInfo() {}
        DexRegisterInfo(uint32_t k, uint32_t v)
            : kind(k), packed_value(v) {}
        DexRegisterInfo(Kind k, uint32_t v)
            : kind(static_cast<uint32_t>(k)), packed_value(v) {}

        static void OatInit124();
        static void OatInit170();

        uint32_t NumColumns() { return kNumDexRegisterInfos; }
        void Decode(BitMemoryReader& reader);
        void Dump(const char* prefix);

        static uint32_t kNumDexRegisterInfos;
        static uint32_t kColNumKind;
        static uint32_t kColNumPackedValue;

        inline uint32_t Kind() const { return kind; }
        inline uint32_t PackedValue() const { return packed_value; }
    private:
        uint32_t kind;
        uint32_t packed_value;
    };

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

    uint32_t NativePc2DexPc(uint32_t native_pc);
    void NativePc2VRegs(uint32_t native_pc, std::map<uint32_t, DexRegisterInfo>& vregs);

    void Dump(const char* prefix);

    static uint32_t kNumHeaders;
    static uint32_t kNumBitTables;
private:
    BitMemoryReader reader = 0;
    uint32_t flags_ = 0;                    // 171+
    uint32_t code_size_ = 0;                // 191+, The size of native PC range in bytes.
    uint32_t packed_frame_size_ = 0;        // 150+, Frame size in kStackAlignment units.
    uint32_t core_spill_mask_ = 0;          // 150+
    uint32_t fp_spill_mask_ = 0;            // 150+
    uint32_t number_of_dex_registers_ = 0;  // 150+
    uint32_t bit_table_flags_ = 0;          // 172+

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
