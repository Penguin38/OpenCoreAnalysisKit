/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef ART_RUNTIME_ARCH_RISCV64_CALLEE_SAVE_FRAME_RISCV64_H_
#define ART_RUNTIME_ARCH_RISCV64_CALLEE_SAVE_FRAME_RISCV64_H_

#include "common/bit.h"
#include "base/macros.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/quick/quick_method_frame_info.h"
#include "runtime/arch/riscv64/registers_riscv64.h"
#include "runtime/runtime_globals.h"

namespace art HIDDEN {
namespace riscv64 {

static constexpr size_t kRiscv64PointerSize = 8;

static constexpr uint32_t kRiscv64CalleeSaveAlwaysSpills =
    (1 << art::riscv64::RA);  // Return address
// Callee-saved registers except for SP and S1 (SP is callee-saved according to RISC-V spec, but
// it cannot contain object reference, and S1(TR) is excluded as the ART thread register).
static constexpr uint32_t kRiscv64CalleeSaveRefSpills =
    (1 << art::riscv64::S0) | (1 << art::riscv64::S2) | (1 << art::riscv64::S3) |
    (1 << art::riscv64::S4) | (1 << art::riscv64::S5) | (1 << art::riscv64::S6) |
    (1 << art::riscv64::S7) | (1 << art::riscv64::S8) | (1 << art::riscv64::S9) |
    (1 << art::riscv64::S10) | (1 << art::riscv64::S11);
// Stack pointer SP is excluded (although it is callee-saved by calling convention) because it is
// restored by the code logic and not from a stack frame.
static constexpr uint32_t kRiscv64CalleeSaveAllSpills = 0;
// Argument registers except X10/A0 (which contains method pointer).
static constexpr uint32_t kRiscv64CalleeSaveArgSpills =
    (1 << art::riscv64::A1) | (1 << art::riscv64::A2) | (1 << art::riscv64::A3) |
    (1 << art::riscv64::A4) | (1 << art::riscv64::A5) | (1 << art::riscv64::A6) |
    (1 << art::riscv64::A7);
// All registers except SP, immutable Zero, unallocatable TP and GP, and the ART thread register TR.
static constexpr uint32_t kRiscv64CalleeSaveEverythingSpills =
    (1 << art::riscv64::T0) | (1 << art::riscv64::T1) | (1 << art::riscv64::T2) |
    (1 << art::riscv64::T3) | (1 << art::riscv64::T4) | (1 << art::riscv64::T5) |
    (1 << art::riscv64::T6) | (1 << art::riscv64::A0) | (1 << art::riscv64::A1) |
    (1 << art::riscv64::A2) | (1 << art::riscv64::A3) | (1 << art::riscv64::A4) |
    (1 << art::riscv64::A5) | (1 << art::riscv64::A6) | (1 << art::riscv64::A7);

// No references in floating-point registers.
static constexpr uint32_t kRiscv64CalleeSaveFpSpills = 0;
// Floating-point argument registers FA0 - FA7.
static constexpr uint32_t kRiscv64CalleeSaveFpArgSpills =
    (1 << art::riscv64::FA0) | (1 << art::riscv64::FA1) | (1 << art::riscv64::FA2) |
    (1 << art::riscv64::FA3) | (1 << art::riscv64::FA4) | (1 << art::riscv64::FA5) |
    (1 << art::riscv64::FA6) | (1 << art::riscv64::FA7);
// Floating-point callee-saved registers FS0 - FS11.
static constexpr uint32_t kRiscv64CalleeSaveFpAllSpills =
    (1 << art::riscv64::FS0) | (1 << art::riscv64::FS1) | (1 << art::riscv64::FS2) |
    (1 << art::riscv64::FS3) | (1 << art::riscv64::FS4) | (1 << art::riscv64::FS5) |
    (1 << art::riscv64::FS6) | (1 << art::riscv64::FS7) | (1 << art::riscv64::FS8) |
    (1 << art::riscv64::FS9) | (1 << art::riscv64::FS10) | (1 << art::riscv64::FS11);
// All floating-point registers.
static constexpr uint32_t kRiscv64CalleeSaveFpEverythingSpills =
    (1 << art::riscv64::FT0) | (1 << art::riscv64::FT1) | (1 << art::riscv64::FT2) |
    (1 << art::riscv64::FT3) | (1 << art::riscv64::FT4) | (1 << art::riscv64::FT5) |
    (1 << art::riscv64::FT6) | (1 << art::riscv64::FT7) | (1 << art::riscv64::FT8) |
    (1 << art::riscv64::FT9) | (1 << art::riscv64::FT10) | (1 << art::riscv64::FT11) |
    (1 << art::riscv64::FS0) | (1 << art::riscv64::FS1) | (1 << art::riscv64::FS2) |
    (1 << art::riscv64::FS3) | (1 << art::riscv64::FS4) | (1 << art::riscv64::FS5) |
    (1 << art::riscv64::FS6) | (1 << art::riscv64::FS7) | (1 << art::riscv64::FS8) |
    (1 << art::riscv64::FS9) | (1 << art::riscv64::FS10) | (1 << art::riscv64::FS11) |
    (1 << art::riscv64::FA0) | (1 << art::riscv64::FA1) | (1 << art::riscv64::FA2) |
    (1 << art::riscv64::FA3) | (1 << art::riscv64::FA4) | (1 << art::riscv64::FA5) |
    (1 << art::riscv64::FA6) | (1 << art::riscv64::FA7);

class Riscv64CalleeSaveFrame {
 public:
  static constexpr uint32_t GetCoreSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kRiscv64CalleeSaveAlwaysSpills | kRiscv64CalleeSaveRefSpills |
           (type == CalleeSaveType::kSaveRefsAndArgs ? kRiscv64CalleeSaveArgSpills : 0) |
           (type == CalleeSaveType::kSaveAllCalleeSaves ? kRiscv64CalleeSaveAllSpills : 0) |
           (type == CalleeSaveType::kSaveEverything ? kRiscv64CalleeSaveEverythingSpills : 0);
  }

  static constexpr uint32_t GetFpSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kRiscv64CalleeSaveFpSpills |
           (type == CalleeSaveType::kSaveRefsAndArgs ? kRiscv64CalleeSaveFpArgSpills : 0) |
           (type == CalleeSaveType::kSaveAllCalleeSaves ? kRiscv64CalleeSaveFpAllSpills : 0) |
           (type == CalleeSaveType::kSaveEverything ? kRiscv64CalleeSaveFpEverythingSpills : 0);
  }

  static constexpr uint32_t GetFrameSize(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return RoundUp((POPCOUNT(GetCoreSpills(type)) /* gprs */ +
                    POPCOUNT(GetFpSpills(type)) /* fprs */ + 1 /* Method* */) *
                       static_cast<size_t>(kRiscv64PointerSize),
                   kStackAlignment);
  }

  static constexpr QuickMethodFrameInfo GetMethodFrameInfo(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return QuickMethodFrameInfo(GetFrameSize(type), GetCoreSpills(type), GetFpSpills(type));
  }

  static constexpr size_t GetFpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) - (POPCOUNT(GetCoreSpills(type)) + POPCOUNT(GetFpSpills(type))) *
                                    static_cast<size_t>(kRiscv64PointerSize);
  }

  static constexpr size_t GetGpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           POPCOUNT(GetCoreSpills(type)) * static_cast<size_t>(kRiscv64PointerSize);
  }

  static constexpr size_t GetReturnPcOffset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) - static_cast<size_t>(kRiscv64PointerSize);
  }
};

// Assembly entrypoints rely on these constants.
static_assert(Riscv64CalleeSaveFrame::GetFrameSize(CalleeSaveType::kSaveRefsAndArgs) == 224);
static_assert(Riscv64CalleeSaveFrame::GetFrameSize(CalleeSaveType::kSaveAllCalleeSaves) == 208);
static_assert(Riscv64CalleeSaveFrame::GetFrameSize(CalleeSaveType::kSaveEverything) == 480);

}  // namespace riscv64
}  // namespace art

#endif  // ART_RUNTIME_ARCH_RISCV64_CALLEE_SAVE_FRAME_RISCV64_H_
