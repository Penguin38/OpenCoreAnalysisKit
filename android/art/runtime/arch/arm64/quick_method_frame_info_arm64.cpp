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

#include "logger/log.h"
#include "api/memory_ref.h"
#include "runtime/arch/arm64/quick_method_frame_info_arm64.h"
#include "runtime/arch/arm64/registers_arm64.h"

namespace art {
namespace arm64 {

std::string Arm64QuickMethodFrameInfo::PrettySpillMask(uint32_t mask) {
    std::string sb;
    uint32_t bits = mask;
    uint32_t regs_size = 0;
    for (uint32_t index = 0; index < XRegister::kNumberOfXRegisters - 1; index++) {
        if(bits & (0x1U << index))
            regs_size++;
    }

    if (regs_size > 0) {
        sb.append("(");
        for (uint32_t index = 0, num = 0; index < XRegister::kNumberOfXRegisters - 1; index++) {
            if (bits & (0x1U << index)) {
                sb.append(XRegisterName(static_cast<XRegister>(index)));
                if (num < regs_size - 1)
                    sb.append(", ");
                num++;
            }
        }
        sb.append(")");
    }
    return sb;
}

void Arm64QuickMethodFrameInfo::DumpCoreSpill(const char* prefix, uint64_t sp, uint32_t core_mask, uint32_t frame_size) {
    uint32_t bits = core_mask;
    uint32_t regs_size = 0;
    for (uint32_t index = 0; index < XRegister::kNumberOfXRegisters - 1; index++) {
        if(bits & (0x1U << index))
            regs_size++;
    }

    api::MemoryRef fp = sp + frame_size - regs_size * 8;
    LOGI("%s{\n", prefix);
    for (uint32_t index = 0, num = 0; index < XRegister::kNumberOfXRegisters - 1; index++) {
        if (bits & (0x1U << index)) {
            if (!(num % 4))
                LOGI("%s    ", prefix);

            LOGI("%s = 0x%016" PRIx64 "", XRegisterName(static_cast<XRegister>(index)), fp.value64Of(num * 8));
            if (num < regs_size - 1) {
                LOGI("    ");
            }
            num++;
            if (!(num % 4))
                ENTER();
        }
    }

    if (regs_size && (regs_size % 4))
        ENTER();
    LOGI("%s}\n", prefix);
}

}  // namespace arm64
}  // namespace art
