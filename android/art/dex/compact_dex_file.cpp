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

#include "dex/compact_dex_file.h"

struct CompactDexFile_CodeItem_OffsetTable __CompactDexFile_CodeItem_offset__;
struct CompactDexFile_CodeItem_SizeTable __CompactDexFile_CodeItem_size__;

namespace art {

void CompactDexFile::Init() {
    art::CompactDexFile::CodeItem::Init();
}

void CompactDexFile::CodeItem::Init() {
    __CompactDexFile_CodeItem_offset__ = {
        .fields_ = 0,
        .insns_count_and_flags_ = 2,
        .insns_ = 4,
    };

    __CompactDexFile_CodeItem_size__ = {
        .THIS = 8,
    };
}

void CompactDexFile::CodeItem::DecodeFields() {
    insns_count_ = insns_count_and_flags() >> CompactDexFile::CodeItem::kInsnsSizeShift;
    num_regs_ = (fields() >> CompactDexFile::CodeItem::kRegistersSizeShift) & 0xF;
    ins_size_ = (fields() >> CompactDexFile::CodeItem::kInsSizeShift) & 0xF;
    out_regs_ = (fields() >> CompactDexFile::CodeItem::kOutsSizeShift) & 0xF;
    tries_size_ = (fields() >> CompactDexFile::CodeItem::kTriesSizeSizeShift) & 0xF;

    if ((insns_count_and_flags() & CompactDexFile::CodeItem::kFlagPreHeaderCombined) != 0x0) {
        api::MemoryRef preheader = Ptr();
        preheader.copyRef(this);
        if ((insns_count_and_flags() & CompactDexFile::CodeItem::kFlagPreHeaderInsnsSize) != 0x0) {
            preheader.MovePtr(-sizeof(uint16_t));
            insns_count_ += static_cast<uint32_t>(preheader.value16Of());
            preheader.MovePtr(-sizeof(uint16_t));
            insns_count_ += static_cast<uint32_t>(preheader.value16Of()) << 16;
        }

        if ((insns_count_and_flags() & CompactDexFile::CodeItem::kFlagPreHeaderRegistersSize) != 0x0) {
            preheader.MovePtr(-sizeof(uint16_t));
            num_regs_ += preheader.value16Of();
        }

        if ((insns_count_and_flags() & CompactDexFile::CodeItem::kFlagPreHeaderInsSize) != 0x0) {
            preheader.MovePtr(-sizeof(uint16_t));
            ins_size_ += preheader.value16Of();
        }

        if ((insns_count_and_flags() & CompactDexFile::CodeItem::kFlagPreHeaderOutsSize) != 0x0) {
            preheader.MovePtr(-sizeof(uint16_t));
            out_regs_ += preheader.value16Of();
        }

        if ((insns_count_and_flags() & CompactDexFile::CodeItem::kFlagPreHeaderTriesSize) != 0x0) {
            preheader.MovePtr(-sizeof(uint16_t));
            tries_size_ += preheader.value16Of();
        }
    }
    num_regs_ += ins_size_;
    code_offset_ = OFFSET(CompactDexFile_CodeItem, insns_);
}

} // namespace art
