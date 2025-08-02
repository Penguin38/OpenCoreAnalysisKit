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

#ifndef ANDROID_ART_DEX_STANDARD_DEX_FILE_H_
#define ANDROID_ART_DEX_STANDARD_DEX_FILE_H_

#include "dex/dex_file.h"

struct StandardDexFile_CodeItem_OffsetTable {
    uint32_t registers_size_;
    uint32_t ins_size_;
    uint32_t outs_size_;
    uint32_t tries_size_;
    uint32_t debug_info_off_;
    uint32_t insns_size_in_code_units_;
    uint32_t insns_;
};

struct StandardDexFile_CodeItem_SizeTable {
    uint32_t THIS;
};

extern struct StandardDexFile_CodeItem_OffsetTable __StandardDexFile_CodeItem_offset__;
extern struct StandardDexFile_CodeItem_SizeTable __StandardDexFile_CodeItem_size__;

namespace art {

class StandardDexFile : public DexFile {
public:
    StandardDexFile(uint64_t v) : DexFile(v) {}
    StandardDexFile(const DexFile& ref) : DexFile(ref) {}
    StandardDexFile(uint64_t v, DexFile* ref) : DexFile(v, ref) {}

    static void Init();

    class CodeItem : public dex::CodeItem {
    public:
        CodeItem(uint64_t v) : dex::CodeItem(v) {}
        CodeItem(const dex::CodeItem& ref) : dex::CodeItem(ref) {}
        CodeItem(uint64_t v, dex::CodeItem* ref) : dex::CodeItem(v, ref) {}

        static void Init();
        inline uint16_t registers_size() { return value16Of(OFFSET(StandardDexFile_CodeItem, registers_size_)); }
        inline uint16_t ins_size() { return value16Of(OFFSET(StandardDexFile_CodeItem, ins_size_)); }
        inline uint16_t outs_size() { return value16Of(OFFSET(StandardDexFile_CodeItem, outs_size_)); }
        inline uint16_t tries_size() { return value16Of(OFFSET(StandardDexFile_CodeItem, tries_size_)); }
        inline uint32_t debug_info_off() { return value32Of(OFFSET(StandardDexFile_CodeItem, debug_info_off_)); }
        inline uint32_t insns_size_in_code_units() { return value32Of(OFFSET(StandardDexFile_CodeItem, insns_size_in_code_units_)); }
        inline uint16_t insns() { return value16Of(OFFSET(StandardDexFile_CodeItem, insns_)); }

        void DecodeFields();
    };
};

} // namespace art

#endif // ANDROID_ART_DEX_STANDARD_DEX_FILE_H_
