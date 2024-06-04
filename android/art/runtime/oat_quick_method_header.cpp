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

#include "api/core.h"
#include "common/elf.h"
#include "android.h"
#include "runtime/oat.h"
#include "runtime/oat_quick_method_header.h"

struct OatQuickMethodHeader_OffsetTable __OatQuickMethodHeader_offset__;
struct OatQuickMethodHeader_SizeTable __OatQuickMethodHeader_size__;

namespace art {

uint32_t OatQuickMethodHeader::kIsCodeInfoMask = 0x40000000;
uint32_t OatQuickMethodHeader::kCodeInfoMask = 0x3FFFFFFF;
uint32_t OatQuickMethodHeader::kCodeSizeMask = 0x3FFFFFFF;

void OatQuickMethodHeader::Init26() {
    __OatQuickMethodHeader_offset__ = {
        .vmap_table_offset_ = 0,
        .method_info_offset_ = 4,
        .frame_info_ = 8,
        .code_size_ = 20,
        .code_ = 24,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 24,
    };
}

void OatQuickMethodHeader::Init29() {
    __OatQuickMethodHeader_offset__ = {
        .vmap_table_offset_ = 0,
        .code_size_ = 4,
        .code_ = 8,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 8,
    };
}

void OatQuickMethodHeader::Init31() {
    __OatQuickMethodHeader_offset__ = {
        .data_ = 0,
        .code_ = 4,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 4,
    };
}

void OatQuickMethodHeader::OatInit() {
    kCodeSizeMask   = ~kShouldDeoptimizeMask;
}

void OatQuickMethodHeader::OatInit195() {
    kIsCodeInfoMask = 0x40000000;
    kCodeInfoMask   = 0x3FFFFFFF;
    kCodeSizeMask   = 0x3FFFFFFF;
}

void OatQuickMethodHeader::OatInit238() {
    kIsCodeInfoMask = 0x80000000;
    kCodeInfoMask   = 0x7FFFFFFF;
    kCodeSizeMask   = 0x7FFFFFFF;
}

bool OatQuickMethodHeader::Contains(uint64_t pc) {
    uint64_t code_start = code();
    if (CoreApi::GetMachine() == EM_AARCH64) {
        code_start &= ((1ULL << 56) - 1);
    } else if (CoreApi::GetMachine() == EM_ARM) {
        code_start++;
    }
    return code_start <= pc && pc <= (code_start + GetCodeSize());
}

bool OatQuickMethodHeader::IsOptimized() {
    return (data() & kIsCodeInfoMask) != 0;
}

uint32_t OatQuickMethodHeader::GetCodeSize() {
    if (OatHeader::OatVersion() >= 195) {
        if (IsOptimized()) {
            return 0x0;
        } else {
            return (data() & kCodeSizeMask);
        }
    } else {
        return code_size() & kCodeSizeMask;
    }
}

QuickMethodFrameInfo OatQuickMethodHeader::GetFrameInfo() {
    QuickMethodFrameInfo frame_info(0, 0, 0);
    return frame_info;
}

} //namespace art
