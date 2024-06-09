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

#ifndef ART_RUNTIME_ARCH_X86_64_CALLEE_SAVE_FRAME_X86_64_H_
#define ART_RUNTIME_ARCH_X86_64_CALLEE_SAVE_FRAME_X86_64_H_

#include "common/bit.h"
#include "base/bit_utils.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/quick/quick_method_frame_info.h"
#include "runtime/arch/x86_64/registers_x86_64.h"
#include "runtime/runtime_globals.h"

namespace art HIDDEN {
namespace x86_64 {

static constexpr size_t kX86_64PointerSize = 8;

static constexpr uint32_t kX86_64CalleeSaveAlwaysSpills =
    (1 << art::x86_64::kNumberOfCpuRegisters);  // Fake return address callee save.
static constexpr uint32_t kX86_64CalleeSaveRefSpills =
    (1 << art::x86_64::RBX) | (1 << art::x86_64::RBP) | (1 << art::x86_64::R12) |
    (1 << art::x86_64::R13) | (1 << art::x86_64::R14) | (1 << art::x86_64::R15);
static constexpr uint32_t kX86_64CalleeSaveArgSpills =
    (1 << art::x86_64::RSI) | (1 << art::x86_64::RDX) | (1 << art::x86_64::RCX) |
    (1 << art::x86_64::R8) | (1 << art::x86_64::R9);
static constexpr uint32_t kX86_64CalleeSaveEverythingSpills =
    (1 << art::x86_64::RAX) | (1 << art::x86_64::RCX) | (1 << art::x86_64::RDX) |
    (1 << art::x86_64::RSI) | (1 << art::x86_64::RDI) | (1 << art::x86_64::R8) |
    (1 << art::x86_64::R9) | (1 << art::x86_64::R10) | (1 << art::x86_64::R11);

static constexpr uint32_t kX86_64CalleeSaveFpArgSpills =
    (1 << art::x86_64::XMM0) | (1 << art::x86_64::XMM1) | (1 << art::x86_64::XMM2) |
    (1 << art::x86_64::XMM3) | (1 << art::x86_64::XMM4) | (1 << art::x86_64::XMM5) |
    (1 << art::x86_64::XMM6) | (1 << art::x86_64::XMM7);
static constexpr uint32_t kX86_64CalleeSaveFpSpills =
    (1 << art::x86_64::XMM12) | (1 << art::x86_64::XMM13) |
    (1 << art::x86_64::XMM14) | (1 << art::x86_64::XMM15);
static constexpr uint32_t kX86_64CalleeSaveFpEverythingSpills =
    (1 << art::x86_64::XMM0) | (1 << art::x86_64::XMM1) |
    (1 << art::x86_64::XMM2) | (1 << art::x86_64::XMM3) |
    (1 << art::x86_64::XMM4) | (1 << art::x86_64::XMM5) |
    (1 << art::x86_64::XMM6) | (1 << art::x86_64::XMM7) |
    (1 << art::x86_64::XMM8) | (1 << art::x86_64::XMM9) |
    (1 << art::x86_64::XMM10) | (1 << art::x86_64::XMM11);

class X86_64CalleeSaveFrame {
 public:
  static constexpr uint32_t GetCoreSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kX86_64CalleeSaveAlwaysSpills | kX86_64CalleeSaveRefSpills |
        (type == CalleeSaveType::kSaveRefsAndArgs ? kX86_64CalleeSaveArgSpills : 0) |
        (type == CalleeSaveType::kSaveEverything ? kX86_64CalleeSaveEverythingSpills : 0);
  }

  static constexpr uint32_t GetFpSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kX86_64CalleeSaveFpSpills |
        (type == CalleeSaveType::kSaveRefsAndArgs ? kX86_64CalleeSaveFpArgSpills : 0) |
        (type == CalleeSaveType::kSaveEverything ? kX86_64CalleeSaveFpEverythingSpills : 0);
  }

  static constexpr uint32_t GetFrameSize(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return RoundUp((POPCOUNT(GetCoreSpills(type)) /* gprs */ +
                    POPCOUNT(GetFpSpills(type)) /* fprs */ +
                    1 /* Method* */) * static_cast<size_t>(kX86_64PointerSize), kStackAlignment);
  }

  static constexpr QuickMethodFrameInfo GetMethodFrameInfo(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return QuickMethodFrameInfo(GetFrameSize(type), GetCoreSpills(type), GetFpSpills(type));
  }

  static constexpr size_t GetFpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           (POPCOUNT(GetCoreSpills(type)) +
            POPCOUNT(GetFpSpills(type))) * static_cast<size_t>(kX86_64PointerSize);
  }

  static constexpr size_t GetGpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           POPCOUNT(GetCoreSpills(type)) * static_cast<size_t>(kX86_64PointerSize);
  }

  static constexpr size_t GetReturnPcOffset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) - static_cast<size_t>(kX86_64PointerSize);
  }
};

}  // namespace x86_64
}  // namespace art

#endif  // ART_RUNTIME_ARCH_X86_64_CALLEE_SAVE_FRAME_X86_64_H_
