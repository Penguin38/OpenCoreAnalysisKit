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
#include "common/elf.h"
#include "runtime/quick/quick_method_frame_info.h"
#include "runtime/arch/arm64/quick_method_frame_info_arm64.h"
#include "runtime/arch/arm/quick_method_frame_info_arm.h"
#include "runtime/arch/x86_64/quick_method_frame_info_x86_64.h"
#include "runtime/arch/x86/quick_method_frame_info_x86.h"
#include "runtime/arch/riscv64/quick_method_frame_info_riscv64.h"

namespace art {

uint32_t QuickMethodFrameInfo::GetReturnPcOffset() const {
    return FrameSizeInBytes() - CoreApi::GetPointSize();
}

std::string QuickMethodFrameInfo::PrettySpillMask(uint32_t mask) {
    int machine = CoreApi::GetMachine();
    switch (machine) {
        case EM_386: return x86::X86QuickMethodFrameInfo::PrettySpillMask(mask);
        case EM_X86_64: return x86_64::X86_64QuickMethodFrameInfo::PrettySpillMask(mask);
        case EM_ARM: return arm::ArmQuickMethodFrameInfo::PrettySpillMask(mask);
        case EM_AARCH64: return arm64::Arm64QuickMethodFrameInfo::PrettySpillMask(mask);
        case EM_RISCV: return riscv64::Riscv64QuickMethodFrameInfo::PrettySpillMask(mask);
    }
    return "";
}

void QuickMethodFrameInfo::DumpCoreSpill(const char* prefix, uint64_t sp) {
    int machine = CoreApi::GetMachine();
    switch (machine) {
        case EM_386:
            x86::X86QuickMethodFrameInfo::DumpCoreSpill(prefix, sp, core_spill_mask_, frame_size_in_bytes_);
            break;
        case EM_X86_64:
            x86_64::X86_64QuickMethodFrameInfo::DumpCoreSpill(prefix, sp, core_spill_mask_, frame_size_in_bytes_);
            break;
        case EM_ARM:
            arm::ArmQuickMethodFrameInfo::DumpCoreSpill(prefix, sp, core_spill_mask_, frame_size_in_bytes_);
            break;
        case EM_AARCH64:
            arm64::Arm64QuickMethodFrameInfo::DumpCoreSpill(prefix, sp, core_spill_mask_, frame_size_in_bytes_);
            break;
        case EM_RISCV:
            riscv64::Riscv64QuickMethodFrameInfo::DumpCoreSpill(prefix, sp, core_spill_mask_, frame_size_in_bytes_);
            break;
    }
}

}  // namespace art
