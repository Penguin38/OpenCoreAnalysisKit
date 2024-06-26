/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef ART_RUNTIME_ARCH_ARM_CALLEE_SAVE_FRAME_ARM_H_
#define ART_RUNTIME_ARCH_ARM_CALLEE_SAVE_FRAME_ARM_H_

#include "common/bit.h"
#include "base/bit_utils.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/quick/quick_method_frame_info.h"
#include "runtime/arch/arm/registers_arm.h"
#include "runtime/runtime_globals.h"

namespace art HIDDEN {
namespace arm {

static constexpr size_t kArmPointerSize = 4;

static constexpr uint32_t kArmCalleeSaveAlwaysSpills =
    (1 << art::arm::LR);
static constexpr uint32_t kArmCalleeSaveRefSpills =
    (1 << art::arm::R5) | (1 << art::arm::R6)  | (1 << art::arm::R7) | (1 << art::arm::R8) |
    (1 << art::arm::R10) | (1 << art::arm::R11);
static constexpr uint32_t kArmCalleeSaveArgSpills =
    (1 << art::arm::R1) | (1 << art::arm::R2) | (1 << art::arm::R3);
static constexpr uint32_t kArmCalleeSaveAllSpills =
    (1 << art::arm::R4) | (1 << art::arm::R9);
static constexpr uint32_t kArmCalleeSaveEverythingSpills =
    (1 << art::arm::R0) | (1 << art::arm::R1) | (1 << art::arm::R2) | (1 << art::arm::R3) |
    (1 << art::arm::R4) | (1 << art::arm::R9) | (1 << art::arm::R12);

static constexpr uint32_t kArmCalleeSaveFpAlwaysSpills = 0;
static constexpr uint32_t kArmCalleeSaveFpRefSpills = 0;
static constexpr uint32_t kArmCalleeSaveFpArgSpills =
    (1 << art::arm::S0)  | (1 << art::arm::S1)  | (1 << art::arm::S2)  | (1 << art::arm::S3)  |
    (1 << art::arm::S4)  | (1 << art::arm::S5)  | (1 << art::arm::S6)  | (1 << art::arm::S7)  |
    (1 << art::arm::S8)  | (1 << art::arm::S9)  | (1 << art::arm::S10) | (1 << art::arm::S11) |
    (1 << art::arm::S12) | (1 << art::arm::S13) | (1 << art::arm::S14) | (1 << art::arm::S15);
static constexpr uint32_t kArmCalleeSaveFpAllSpills =
    (1 << art::arm::S16) | (1 << art::arm::S17) | (1 << art::arm::S18) | (1 << art::arm::S19) |
    (1 << art::arm::S20) | (1 << art::arm::S21) | (1 << art::arm::S22) | (1 << art::arm::S23) |
    (1 << art::arm::S24) | (1 << art::arm::S25) | (1 << art::arm::S26) | (1 << art::arm::S27) |
    (1 << art::arm::S28) | (1 << art::arm::S29) | (1 << art::arm::S30) | (1 << art::arm::S31);
static constexpr uint32_t kArmCalleeSaveFpEverythingSpills =
    kArmCalleeSaveFpArgSpills | kArmCalleeSaveFpAllSpills;

class ArmCalleeSaveFrame {
 public:
  static constexpr uint32_t GetCoreSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kArmCalleeSaveAlwaysSpills | kArmCalleeSaveRefSpills |
        (type == CalleeSaveType::kSaveRefsAndArgs ? kArmCalleeSaveArgSpills : 0) |
        (type == CalleeSaveType::kSaveAllCalleeSaves ? kArmCalleeSaveAllSpills : 0) |
        (type == CalleeSaveType::kSaveEverything ? kArmCalleeSaveEverythingSpills : 0);
  }

  static constexpr uint32_t GetFpSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kArmCalleeSaveFpAlwaysSpills | kArmCalleeSaveFpRefSpills |
        (type == CalleeSaveType::kSaveRefsAndArgs ? kArmCalleeSaveFpArgSpills : 0) |
        (type == CalleeSaveType::kSaveAllCalleeSaves ? kArmCalleeSaveFpAllSpills : 0) |
        (type == CalleeSaveType::kSaveEverything ? kArmCalleeSaveFpEverythingSpills : 0);
  }

  static constexpr uint32_t GetFrameSize(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return RoundUp((POPCOUNT(GetCoreSpills(type)) /* gprs */ +
                    POPCOUNT(GetFpSpills(type)) /* fprs */ +
                    1 /* Method* */) * static_cast<size_t>(kArmPointerSize), kStackAlignment);
  }

  static constexpr QuickMethodFrameInfo GetMethodFrameInfo(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return QuickMethodFrameInfo(GetFrameSize(type), GetCoreSpills(type), GetFpSpills(type));
  }

  static constexpr size_t GetFpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           (POPCOUNT(GetCoreSpills(type)) +
            POPCOUNT(GetFpSpills(type))) * static_cast<size_t>(kArmPointerSize);
  }

  static constexpr size_t GetGpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           POPCOUNT(GetCoreSpills(type)) * static_cast<size_t>(kArmPointerSize);
  }

  static constexpr size_t GetReturnPcOffset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) - static_cast<size_t>(kArmPointerSize);
  }
};

}  // namespace arm
}  // namespace art

#endif  // ART_RUNTIME_ARCH_ARM_CALLEE_SAVE_FRAME_ARM_H_
