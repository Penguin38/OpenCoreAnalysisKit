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

#ifndef ANDROID_ART_RUNTIME_INTERPRETER_SHADOW_FRAME_H_
#define ANDROID_ART_RUNTIME_INTERPRETER_SHADOW_FRAME_H_

#include "api/memory_ref.h"
#include "runtime/art_method.h"
#include "runtime/oat/stack_map.h"

struct ShadowFrame_OffsetTable {
    uint32_t link_;
    uint32_t method_;
    uint32_t result_register_;
    uint32_t dex_pc_ptr_;
    uint32_t dex_instructions_;
    uint32_t lock_count_data_;
    uint32_t number_of_vregs_;
    uint32_t dex_pc_;
    uint32_t cached_hotness_countdown_;
    uint32_t hotness_countdown_;
    uint32_t frame_flags_;
    uint32_t vregs_;
};

struct ShadowFrame_SizeTable {
    uint32_t THIS;
};

extern struct ShadowFrame_OffsetTable __ShadowFrame_offset__;
extern struct ShadowFrame_SizeTable __ShadowFrame_size__;

namespace art {

class ShadowFrame : public api::MemoryRef {
public:
    ShadowFrame(uint64_t v) : api::MemoryRef(v) {}
    ShadowFrame(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ShadowFrame(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ShadowFrame(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t link() { return VALUEOF(ShadowFrame, link_); }
    inline uint64_t method() { return VALUEOF(ShadowFrame, method_); }
    inline uint64_t dex_pc_ptr() { return VALUEOF(ShadowFrame, dex_pc_ptr_); }
    inline uint32_t number_of_vregs() { return value32Of(OFFSET(ShadowFrame, number_of_vregs_)); }
    inline uint32_t dex_pc() { return value32Of(OFFSET(ShadowFrame, dex_pc_)); }
    inline uint64_t vregs() { return Ptr() + OFFSET(ShadowFrame, vregs_); }

    inline ArtMethod GetMethod() { return method(); }
    uint64_t GetDexPcPtr();
    std::map<uint32_t, CodeInfo::DexRegisterInfo>& GetVRegs();
private:
    std::map<uint32_t, CodeInfo::DexRegisterInfo> vregs_cache;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_INTERPRETER_SHADOW_FRAME_H_
