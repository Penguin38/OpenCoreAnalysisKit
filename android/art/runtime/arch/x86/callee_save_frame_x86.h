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

#ifndef ART_RUNTIME_ARCH_X86_CALLEE_SAVE_FRAME_X86_H_
#define ART_RUNTIME_ARCH_X86_CALLEE_SAVE_FRAME_X86_H_

#include "common/bit.h"
#include "base/macros.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/quick/quick_method_frame_info.h"
#include "runtime/arch/x86/registers_x86.h"
#include "runtime/runtime_globals.h"

namespace art HIDDEN {
namespace x86 {

static constexpr size_t kX86PointerSize = 4;

static constexpr uint32_t kX86CalleeSaveAlwaysSpills =
    (1 << art::x86::kNumberOfCpuRegisters);  // Fake return address callee save.
static constexpr uint32_t kX86CalleeSaveRefSpills =
    (1 << art::x86::EBP) | (1 << art::x86::ESI) | (1 << art::x86::EDI);
static constexpr uint32_t kX86CalleeSaveArgSpills =
    (1 << art::x86::ECX) | (1 << art::x86::EDX) | (1 << art::x86::EBX);
static constexpr uint32_t kX86CalleeSaveEverythingSpills =
    (1 << art::x86::EAX) | (1 << art::x86::ECX) | (1 << art::x86::EDX) | (1 << art::x86::EBX);

static constexpr uint32_t kX86CalleeSaveFpArgSpills =
    (1 << art::x86::XMM0) | (1 << art::x86::XMM1) |
    (1 << art::x86::XMM2) | (1 << art::x86::XMM3);
static constexpr uint32_t kX86CalleeSaveFpEverythingSpills =
    (1 << art::x86::XMM0) | (1 << art::x86::XMM1) |
    (1 << art::x86::XMM2) | (1 << art::x86::XMM3) |
    (1 << art::x86::XMM4) | (1 << art::x86::XMM5) |
    (1 << art::x86::XMM6) | (1 << art::x86::XMM7);

class X86CalleeSaveFrame {
 public:
  static constexpr uint32_t GetCoreSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return kX86CalleeSaveAlwaysSpills | kX86CalleeSaveRefSpills |
        (type == CalleeSaveType::kSaveRefsAndArgs ? kX86CalleeSaveArgSpills : 0) |
        (type == CalleeSaveType::kSaveEverything ? kX86CalleeSaveEverythingSpills : 0);
  }

  static constexpr uint32_t GetFpSpills(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return (type == CalleeSaveType::kSaveRefsAndArgs ? kX86CalleeSaveFpArgSpills : 0) |
           (type == CalleeSaveType::kSaveEverything ? kX86CalleeSaveFpEverythingSpills : 0);
  }

  static constexpr uint32_t GetFrameSize(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return RoundUp((POPCOUNT(GetCoreSpills(type)) /* gprs */ +
                    2 * POPCOUNT(GetFpSpills(type)) /* fprs */ +
                    1 /* Method* */) * static_cast<size_t>(kX86PointerSize), kStackAlignment);
  }

  static constexpr QuickMethodFrameInfo GetMethodFrameInfo(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return QuickMethodFrameInfo(GetFrameSize(type), GetCoreSpills(type), GetFpSpills(type));
  }

  static constexpr size_t GetFpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           (POPCOUNT(GetCoreSpills(type)) +
            2 * POPCOUNT(GetFpSpills(type))) * static_cast<size_t>(kX86PointerSize);
  }

  static constexpr size_t GetGpr1Offset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) -
           POPCOUNT(GetCoreSpills(type)) * static_cast<size_t>(kX86PointerSize);
  }

  static constexpr size_t GetReturnPcOffset(CalleeSaveType type) {
    type = GetCanonicalCalleeSaveType(type);
    return GetFrameSize(type) - static_cast<size_t>(kX86PointerSize);
  }
};

}  // namespace x86
}  // namespace art

#endif  // ART_RUNTIME_ARCH_X86_CALLEE_SAVE_FRAME_X86_H_
