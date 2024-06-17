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
#include "runtime/nterp_helpers.h"
#include "runtime/art_method.h"
#include "runtime/runtime_globals.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/entrypoints/quick/callee_save_frame.h"
#include "runtime/arch/arm64/callee_save_frame_arm64.h"
#include "runtime/arch/arm/callee_save_frame_arm.h"
#include "runtime/arch/x86_64/callee_save_frame_x86_64.h"
#include "runtime/arch/x86/callee_save_frame_x86.h"
#include "runtime/arch/riscv64/callee_save_frame_riscv64.h"
#include "runtime/oat/stack_map.h"
#include "base/globals.h"
#include "common/bit.h"

namespace art {

static uint64_t NterpGetFrameEntrySize() {
    int machine = CoreApi::GetMachine();
    uint32_t core_spills = 0;
    uint32_t fp_spills = 0;
    switch (machine) {
        case EM_386:
            core_spills = x86::X86CalleeSaveFrame::GetCoreSpills(CalleeSaveType::kSaveAllCalleeSaves);
            fp_spills = x86::X86CalleeSaveFrame::GetFpSpills(CalleeSaveType::kSaveAllCalleeSaves);
            break;
        case EM_X86_64:
            core_spills = x86_64::X86_64CalleeSaveFrame::GetCoreSpills(CalleeSaveType::kSaveAllCalleeSaves);
            fp_spills = x86_64::X86_64CalleeSaveFrame::GetFpSpills(CalleeSaveType::kSaveAllCalleeSaves);
            break;
        case EM_ARM:
            core_spills = arm::ArmCalleeSaveFrame::GetCoreSpills(CalleeSaveType::kSaveAllCalleeSaves);
            fp_spills = arm::ArmCalleeSaveFrame::GetFpSpills(CalleeSaveType::kSaveAllCalleeSaves);
            break;
        case EM_AARCH64:
            core_spills = arm64::Arm64CalleeSaveFrame::GetCoreSpills(CalleeSaveType::kSaveAllCalleeSaves);
            fp_spills = arm64::Arm64CalleeSaveFrame::GetFpSpills(CalleeSaveType::kSaveAllCalleeSaves);
            break;
        case EM_RISCV:
            core_spills = riscv64::Riscv64CalleeSaveFrame::GetCoreSpills(CalleeSaveType::kSaveAllCalleeSaves);
            fp_spills = riscv64::Riscv64CalleeSaveFrame::GetFpSpills(CalleeSaveType::kSaveAllCalleeSaves);
            break;
    }
    return (__builtin_popcount(core_spills) + __builtin_popcount(fp_spills))
           * (CoreApi::GetPointSize());
}

static uint64_t NterpGetFrameSizeWithoutPadding(ArtMethod& method) {
    art::dex::CodeItem item = method.GetCodeItem();
    const uint16_t num_regs = item.num_regs_;
    const uint16_t out_regs = item.out_regs_;
    uint32_t pointer_size = CoreApi::GetPointSize();

    uint64_t frame_size =
            NterpGetFrameEntrySize() +
            (num_regs * kVRegSize) * 2 + // dex registers and reference registers
            pointer_size + // previous frame
            pointer_size + // saved dex pc
            (out_regs * kVRegSize) + // out arguments
            pointer_size;  // method

    return frame_size;
}

static inline uint64_t NterpGetFrameSize(ArtMethod& method) {
    return RoundUp(NterpGetFrameSizeWithoutPadding(method), kStackAlignment);
}

QuickMethodFrameInfo NterpFrameInfo(ArtMethod& method) {
    uint32_t core_spills =
        RuntimeCalleeSaveFrame::GetCoreSpills(CalleeSaveType::kSaveAllCalleeSaves);
    uint32_t fp_spills =
        RuntimeCalleeSaveFrame::GetFpSpills(CalleeSaveType::kSaveAllCalleeSaves);
    return QuickMethodFrameInfo(NterpGetFrameSize(method), core_spills, fp_spills);
}

uint64_t NterpGetFrameDexPcPtr(QuickFrame& frame) {
    ArtMethod& method = frame.GetMethod();
    art::dex::CodeItem item = method.GetCodeItem();
    const uint16_t num_regs = item.num_regs_;
    const uint16_t out_regs = item.out_regs_;
    uint32_t pointer_size = CoreApi::GetPointSize();

    api::MemoryRef dex_pc_ptr(frame.Ptr() +
                              pointer_size +
                              RoundUp(out_regs * kVRegSize, pointer_size),
                              frame);
    return dex_pc_ptr.valueOf();
}

void NterpGetFrameVRegs(QuickFrame& frame) {
    std::map<uint32_t, CodeInfo::DexRegisterInfo>& vregs = frame.GetVRegsCache();
    ArtMethod& method = frame.GetMethod();
    art::dex::CodeItem item = method.GetCodeItem();
    const uint16_t num_regs = item.num_regs_;
    const uint16_t out_regs = item.out_regs_;
    uint32_t pointer_size = CoreApi::GetPointSize();

    api::MemoryRef dex_vregs_ptr(frame.Ptr() +
                                 pointer_size +
                                 RoundUp(out_regs * kVRegSize, pointer_size) +
                                 pointer_size +
                                 pointer_size +
                                 (num_regs * kVRegSize),
                                 frame);

    for (int i = 0; i < num_regs; ++i) {
        CodeInfo::DexRegisterInfo info(CodeInfo::DexRegisterInfo::Kind::kConstant,
                                       dex_vregs_ptr.value32Of(i * sizeof(uint32_t)));
        vregs[i] = info;
    }
}

} // namespace art
