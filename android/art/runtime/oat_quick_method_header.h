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

#ifndef ANDROID_ART_RUNTIME_OAT_QUICK_METHOD_HEADER_H_
#define ANDROID_ART_RUNTIME_OAT_QUICK_METHOD_HEADER_H_

#include "api/memory_ref.h"
#include "runtime/quick/quick_method_frame_info.h"

struct OatQuickMethodHeader_OffsetTable {
    uint32_t code_info_offset_;
    uint32_t data_;
    uint32_t vmap_table_offset_;
    uint32_t method_info_offset_;
    uint32_t frame_info_;
    uint32_t code_size_;
    uint32_t code_;
};

struct OatQuickMethodHeader_SizeTable {
    uint32_t THIS;
};

extern struct OatQuickMethodHeader_OffsetTable __OatQuickMethodHeader_offset__;
extern struct OatQuickMethodHeader_SizeTable __OatQuickMethodHeader_size__;

namespace art {

class OatQuickMethodHeader : public api::MemoryRef {
public:
    OatQuickMethodHeader(uint64_t v) : api::MemoryRef(v) {}
    OatQuickMethodHeader(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    OatQuickMethodHeader(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    OatQuickMethodHeader(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> OatQuickMethodHeader(U *v) : api::MemoryRef(v) {}
    template<typename U> OatQuickMethodHeader(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(OatQuickMethodHeader& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(OatQuickMethodHeader& ref) { return Ptr() != ref.Ptr(); }

    static void Init26();
    static void Init29();
    static void Init31();
    inline uint32_t code_info_offset() { return value32Of(OFFSET(OatQuickMethodHeader, code_info_offset_)); }
    inline uint32_t data() { return value32Of(OFFSET(OatQuickMethodHeader, data_)); }
    inline uint32_t vmap_table_offset() { return value32Of(OFFSET(OatQuickMethodHeader, vmap_table_offset_)); }
    inline uint32_t method_info_offset() { return value32Of(OFFSET(OatQuickMethodHeader, method_info_offset_)); }
    inline uint64_t frame_info() { return Ptr() + OFFSET(OatQuickMethodHeader, frame_info_); }
    inline uint32_t code_size() { return value32Of(OFFSET(OatQuickMethodHeader, code_size_)); }
    inline uint64_t code() { return Ptr() + OFFSET(OatQuickMethodHeader, code_); }

    static constexpr uint32_t kShouldDeoptimizeMask = 0x80000000;

    static void OatInit124(); // 8.0.0_r1 Base
    static void OatInit156(); // Remove frame info from OatQuickMethodHeader.
    static void OatInit158(); // Move MethodInfo to CodeInfo.
    static void OatInit192(); // Move code size from OatQuickMethodHeader to CodeInfo.
    static void OatInit238(); // Move HasShouldDeoptimizeFlag from method header to CodeInfo.
    static void OatInit239(); // Refactor OatQuickMethodHeader for assembly stubs.

    static uint32_t kIsCodeInfoMask;
    static uint32_t kCodeInfoMask;
    static uint32_t kCodeSizeMask;

    static OatQuickMethodHeader FromEntryPoint(uint64_t entry_point) {
        return FromCodePointer(entry_point & ~0x1UL);
    }
    static OatQuickMethodHeader FromCodePointer(uint64_t code_ptr) {
        OatQuickMethodHeader header = code_ptr - OFFSET(OatQuickMethodHeader, code_);
        return header;
    }

    bool Contains(uint64_t pc);
    bool IsOptimized();
    uint32_t GetCodeSize();
    QuickMethodFrameInfo GetFrameInfo();
    uint32_t GetCodeInfoOffset();
    uint64_t GetOptimizedCodeInfoPtr() { return code() - GetCodeInfoOffset(); }
    static bool IsNterpPc(uint64_t pc);
    static OatQuickMethodHeader GetNterpMethodHeader();
    uint64_t NativePc2DexPc(uint64_t pc);
private:
    // quick memoryref cache

};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_OAT_QUICK_METHOD_HEADER_H_
