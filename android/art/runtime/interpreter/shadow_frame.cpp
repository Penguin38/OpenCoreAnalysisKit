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

#include "api/core.h"
#include "android.h"
#include "runtime/interpreter/shadow_frame.h"

struct ShadowFrame_OffsetTable __ShadowFrame_offset__;
struct ShadowFrame_SizeTable __ShadowFrame_size__;

namespace art {

void ShadowFrame::Init() {
    if (CoreApi::Bits() == 64) {
        __ShadowFrame_offset__ = {
            .link_ = 0,
            .method_ = 8,
            .result_register_ = 16,
            .dex_pc_ptr_ = 24,
            .dex_instructions_ = 32,
            .lock_count_data_ = 40,
            .number_of_vregs_ = 48,
            .dex_pc_ = 52,
            .cached_hotness_countdown_ = 56,
            .hotness_countdown_ = 58,
            .frame_flags_ = 60,
            .vregs_ = 64,
        };

        __ShadowFrame_size__ = {
            .THIS = 64,
        };
    } else {
        __ShadowFrame_offset__ = {
            .link_ = 0,
            .method_ = 4,
            .result_register_ = 8,
            .dex_pc_ptr_ = 12,
            .dex_instructions_ = 16,
            .lock_count_data_ = 20,
            .number_of_vregs_ = 24,
            .dex_pc_ = 28,
            .cached_hotness_countdown_ = 28,
            .hotness_countdown_ = 32,
            .frame_flags_ = 36,
            .vregs_ = 40,
        };

        __ShadowFrame_size__ = {
            .THIS = 40,
        };
    }
}

uint64_t ShadowFrame::GetDexPcPtr() {
    if (!dex_pc_ptr()) {
        ArtMethod method = GetMethod();
        art::dex::CodeItem item = method.GetCodeItem();
        if (item.Ptr()) return (item.Ptr() + item.code_offset_ + 0x2 * dex_pc());
    }
    return dex_pc_ptr();
}

std::map<uint32_t, DexRegisterInfo>& ShadowFrame::GetVRegs() {
    if (!vregs_cache.size()) {
        api::MemoryRef ref = vregs();
        for (int i = 0; i < number_of_vregs(); i++) {
            DexRegisterInfo info(DexRegisterInfo::Kind::kConstant,
                                           ref.value32Of(i * sizeof(uint32_t)));
            if (Android::Sdk() >= Android::P) {
                vregs_cache[i] = info;
            } else {
                vregs_cache[i + 1]  = info;
            }
        }
    }
    return vregs_cache;
}

} //namespace art
