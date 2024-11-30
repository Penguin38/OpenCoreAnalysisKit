/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_ART_DEX_COMPACT_DEX_FILE_H_
#define ANDROID_ART_DEX_COMPACT_DEX_FILE_H_

#include "dex/dex_file.h"
#include "base/globals.h"

struct CompactDexFile_CodeItem_OffsetTable {
    uint32_t fields_;
    uint32_t insns_count_and_flags_;
    uint32_t insns_;
};

struct CompactDexFile_CodeItem_SizeTable {
    uint32_t THIS;
    uint32_t fields_;
    uint32_t insns_count_and_flags_;
    uint32_t insns_;
};

extern struct CompactDexFile_CodeItem_OffsetTable __CompactDexFile_CodeItem_offset__;
extern struct CompactDexFile_CodeItem_SizeTable __CompactDexFile_CodeItem_size__;

namespace art {

class CompactDexFile : public DexFile {
public:
    CompactDexFile(uint64_t v) : DexFile(v) {}
    CompactDexFile(const DexFile& ref) : DexFile(ref) {}
    CompactDexFile(uint64_t v, DexFile* ref) : DexFile(v, ref) {}

    static void Init();

    class CodeItem : public dex::CodeItem {
    public:
        CodeItem(uint64_t v) : dex::CodeItem(v) {}
        CodeItem(const dex::CodeItem& ref) : dex::CodeItem(ref) {}
        CodeItem(uint64_t v, dex::CodeItem* ref) : dex::CodeItem(v, ref) {}

        static constexpr uint64_t kRegistersSizeShift = 12;
        static constexpr uint64_t kInsSizeShift = 8;
        static constexpr uint64_t kOutsSizeShift = 4;
        static constexpr uint64_t kTriesSizeSizeShift = 0;
        static constexpr uint16_t kBitPreHeaderRegistersSize = 0;
        static constexpr uint16_t kBitPreHeaderInsSize = 1;
        static constexpr uint16_t kBitPreHeaderOutsSize = 2;
        static constexpr uint16_t kBitPreHeaderTriesSize = 3;
        static constexpr uint16_t kBitPreHeaderInsnsSize = 4;
        static constexpr uint16_t kFlagPreHeaderRegistersSize = 0x1 << kBitPreHeaderRegistersSize;
        static constexpr uint16_t kFlagPreHeaderInsSize = 0x1 << kBitPreHeaderInsSize;
        static constexpr uint16_t kFlagPreHeaderOutsSize = 0x1 << kBitPreHeaderOutsSize;
        static constexpr uint16_t kFlagPreHeaderTriesSize = 0x1 << kBitPreHeaderTriesSize;
        static constexpr uint16_t kFlagPreHeaderInsnsSize = 0x1 << kBitPreHeaderInsnsSize;
        static constexpr uint64_t kInsnsSizeShift = 5;
        static constexpr uint64_t kInsnsSizeBits = sizeof(uint16_t) * kBitsPerByte -  kInsnsSizeShift;

        static constexpr uint16_t kFlagPreHeaderCombined =
            kFlagPreHeaderRegistersSize |
            kFlagPreHeaderInsSize |
            kFlagPreHeaderOutsSize |
            kFlagPreHeaderTriesSize |
            kFlagPreHeaderInsnsSize;

        static void Init();
        inline uint16_t fields() { return value16Of(OFFSET(CompactDexFile_CodeItem, fields_)); }
        inline uint16_t insns_count_and_flags() { return value16Of(OFFSET(CompactDexFile_CodeItem, insns_count_and_flags_)); }
        inline uint16_t insns() { return value16Of(OFFSET(CompactDexFile_CodeItem, insns_)); }

        void DecodeFields();
    };
};

} // namespace art

#endif // ANDROID_ART_DEX_COMPACT_DEX_FILE_H_
