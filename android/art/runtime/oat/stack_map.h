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

namespace art {

class CodeInfo {
public:
    static CodeInfo DecodeHeaderOnly(uint64_t code_info_data);
    static uint32_t DecodeCodeSize(uint64_t code_info_data);
    static QuickMethodFrameInfo DecodeFrameInfo(uint64_t code_info_data);

private:
    uint32_t flags_ = 0;
    uint32_t code_size_ = 0;  // The size of native PC range in bytes.
    uint32_t packed_frame_size_ = 0;  // Frame size in kStackAlignment units.
    uint32_t core_spill_mask_ = 0;
    uint32_t fp_spill_mask_ = 0;
    uint32_t number_of_dex_registers_ = 0;
    uint32_t bit_table_flags_ = 0;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_OAT_STACK_MAP_H_
