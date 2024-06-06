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
#include "runtime/oat/stack_map.h"

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

void OatQuickMethodHeader::OatInit124() {
    kCodeSizeMask   = ~kShouldDeoptimizeMask;

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

void OatQuickMethodHeader::OatInit156() {
    __OatQuickMethodHeader_offset__ = {
        .vmap_table_offset_ = 0,
        .method_info_offset_ = 4,
        .code_size_ = 8,
        .code_ = 12,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 12,
    };
}

void OatQuickMethodHeader::OatInit158() {
    __OatQuickMethodHeader_offset__ = {
        .vmap_table_offset_ = 0,
        .code_size_ = 4,
        .code_ = 8,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 8,
    };
}

void OatQuickMethodHeader::OatInit192() {
    kIsCodeInfoMask = 0x40000000;
    kCodeInfoMask   = 0x3FFFFFFF;
    kCodeSizeMask   = 0x3FFFFFFF;

    __OatQuickMethodHeader_offset__ = {
        .data_ = 0,
        .code_ = 4,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 4,
    };
}

void OatQuickMethodHeader::OatInit238() {
    kIsCodeInfoMask = 0x80000000;
    kCodeInfoMask   = 0x7FFFFFFF;
    kCodeSizeMask   = 0x7FFFFFFF;
}

void OatQuickMethodHeader::OatInit239() {
    __OatQuickMethodHeader_offset__ = {
        .code_info_offset_ = 0,
        .code_ = 4,
    };

    __OatQuickMethodHeader_size__ = {
        .THIS = 4,
    };
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
    if (OatHeader::OatVersion() >= 192) {
        return (data() & kIsCodeInfoMask) != 0;
    } else {
        return GetCodeSize() != 0 && vmap_table_offset() != 0;
    }
}

uint32_t OatQuickMethodHeader::GetCodeSize() {
    if (OatHeader::OatVersion() > 238) {
        //TODO
        return CodeInfo::DecodeCodeSize(GetOptimizedCodeInfoPtr());
    } else if (OatHeader::OatVersion() >= 192) {
        if (IsOptimized()) {
            return CodeInfo::DecodeCodeSize(GetOptimizedCodeInfoPtr());
        } else {
            return (data() & kCodeSizeMask);
        }
    } else {
        return code_size() & kCodeSizeMask;
    }
}

uint32_t OatQuickMethodHeader::GetCodeInfoOffset() {
    if (OatHeader::OatVersion() < 239) {
        return data() & kCodeInfoMask;
    } else {
        return code_info_offset();
    }
}

QuickMethodFrameInfo OatQuickMethodHeader::GetFrameInfo() {
    if (OatHeader::OatVersion() >= 192) {
        return CodeInfo::DecodeFrameInfo(GetOptimizedCodeInfoPtr());
    } else {
        api::MemoryRef ref(frame_info(), this);
        return QuickMethodFrameInfo(ref.value32Of(), ref.value32Of(4), ref.value32Of(8));
    }
}

bool OatQuickMethodHeader::IsNterpPc(uint64_t pc) {
    OatQuickMethodHeader NterpMethodHeader = GetNterpMethodHeader();
    return NterpMethodHeader.Ptr() && NterpMethodHeader.Contains(pc);
}

OatQuickMethodHeader OatQuickMethodHeader::GetNterpMethodHeader() {
    OatQuickMethodHeader header = 0x0;
    try {
        api::MemoryRef value = Android::SearchSymbol(Android::NTERP_METHOD_HEADER);
        header = value.valueOf();
    } catch(InvalidAddressException e) {
    }

    if (header.Ptr()) return header;

    uint64_t entry_point = Android::SearchSymbol(Android::EXECUTE_NTERP_IMPL);
    header = OatQuickMethodHeader::FromEntryPoint(entry_point);
    return header;
}

} //namespace art
