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

#include "dex/standard_dex_file.h"

struct StandardDexFile_CodeItem_OffsetTable __StandardDexFile_CodeItem_offset__;
struct StandardDexFile_CodeItem_SizeTable __StandardDexFile_CodeItem_size__;

namespace art {

void StandardDexFile::CodeItem::Init() {
    __StandardDexFile_CodeItem_offset__ = {
        .registers_size_ = 0,
        .ins_size_ = 2,
        .outs_size_ = 4,
        .tries_size_ = 6,
        .debug_info_off_ = 8,
        .insns_size_in_code_units_ = 12,
        .insns_ = 16,
    };

    __StandardDexFile_CodeItem_size__ = {
        .THIS = 20,
        .registers_size_ = 2,
        .ins_size_ = 2,
        .outs_size_ = 2,
        .tries_size_ = 2,
        .debug_info_off_ = 4,
        .insns_size_in_code_units_ = 4,
        .insns_ = 2,
    };
}

void StandardDexFile::CodeItem::DecodeFields() {
    num_regs_ = registers_size();
    out_regs_ = outs_size();
    insns_count_ = insns_size_in_code_units();
    ins_size_ = ins_size();
    tries_size_ = tries_size();
    code_offset_ = OFFSET(StandardDexFile_CodeItem, insns_);
}

} // namespace art
