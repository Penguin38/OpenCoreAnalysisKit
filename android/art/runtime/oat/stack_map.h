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

namespace art {

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

    static void OatInit124(); // 8.0.0_r1 Base
    static void OatInit150(); // Add method frame info to CodeInfo.
    static void OatInit171(); // Optimize stack maps: add fast path for no inline info.
    static void OatInit172(); // Stack maps: Handle special cases using flags.
    static void OatInit191(); // Add code size to CodeInfo.

    bool HasBitTable(uint32_t i ) { return ((bit_table_flags_ >> i) & 1) != 0; }
    bool IsBitTableDeduped(uint32_t i) { return ((bit_table_flags_ >> (kNumBitTables + i)) & 1) != 0; }
    bool HasDedupedBitTables() { return (bit_table_flags_ >> kNumBitTables) != 0u; }

    BitMemoryReader& GetMemoryReader() { return reader; }
    StackMap& GetStackMap() { return stack_map_; }
    RegisterMask& GetRegisterMask() { return register_mask_; }

    uint32_t NativePc2DexPc(uint32_t native_pc);

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
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_OAT_STACK_MAP_H_
