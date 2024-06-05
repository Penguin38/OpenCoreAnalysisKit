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

#ifndef ART_RUNTIME_ARCH_RISCV64_REGISTERS_RISCV64_H_
#define ART_RUNTIME_ARCH_RISCV64_REGISTERS_RISCV64_H_

#include <iosfwd>

#include "base/macros.h"

namespace art HIDDEN {
namespace riscv64 {

enum XRegister {
  Zero = 0,  // X0, hard-wired zero
  RA = 1,    // X1, return address
  SP = 2,    // X2, stack pointer
  GP = 3,    // X3, global pointer (unavailable, used for shadow stack by the compiler / libc)
  TP = 4,    // X4, thread pointer (points to TLS area, not ART-internal thread)

  T0 = 5,  // X5, temporary 0
  T1 = 6,  // X6, temporary 1
  T2 = 7,  // X7, temporary 2

  S0 = 8,  // X8/FP, callee-saved 0 / frame pointer
  S1 = 9,  // X9, callee-saved 1 / ART thread register

  A0 = 10,  // X10, argument 0 / return value 0
  A1 = 11,  // X11, argument 1 / return value 1
  A2 = 12,  // X12, argument 2
  A3 = 13,  // X13, argument 3
  A4 = 14,  // X14, argument 4
  A5 = 15,  // X15, argument 5
  A6 = 16,  // X16, argument 6
  A7 = 17,  // X17, argument 7

  S2 = 18,   // X18, callee-saved 2
  S3 = 19,   // X19, callee-saved 3
  S4 = 20,   // X20, callee-saved 4
  S5 = 21,   // X21, callee-saved 5
  S6 = 22,   // X22, callee-saved 6
  S7 = 23,   // X23, callee-saved 7
  S8 = 24,   // X24, callee-saved 8
  S9 = 25,   // X25, callee-saved 9
  S10 = 26,  // X26, callee-saved 10
  S11 = 27,  // X27, callee-saved 11

  T3 = 28,  // X28, temporary 3
  T4 = 29,  // X29, temporary 4
  T5 = 30,  // X30, temporary 5
  T6 = 31,  // X31, temporary 6

  kNumberOfXRegisters = 32,
  kNoXRegister = -1,  // Signals an illegal X register.

  // Aliases.
  TR = S1,    // ART Thread Register - managed runtime
  TMP = T6,   // Reserved for special uses, such as assembler macro instructions.
  TMP2 = T5,  // Reserved for special uses, such as assembler macro instructions.
};

std::ostream& operator<<(std::ostream& os, const XRegister& rhs);

enum FRegister {
  FT0 = 0,  // F0, temporary 0
  FT1 = 1,  // F1, temporary 1
  FT2 = 2,  // F2, temporary 2
  FT3 = 3,  // F3, temporary 3
  FT4 = 4,  // F4, temporary 4
  FT5 = 5,  // F5, temporary 5
  FT6 = 6,  // F6, temporary 6
  FT7 = 7,  // F7, temporary 7

  FS0 = 8,  // F8, callee-saved 0
  FS1 = 9,  // F9, callee-saved 1

  FA0 = 10,  // F10, argument 0 / return value 0
  FA1 = 11,  // F11, argument 1 / return value 1
  FA2 = 12,  // F12, argument 2
  FA3 = 13,  // F13, argument 3
  FA4 = 14,  // F14, argument 4
  FA5 = 15,  // F15, argument 5
  FA6 = 16,  // F16, argument 6
  FA7 = 17,  // F17, argument 7

  FS2 = 18,   // F18, callee-saved 2
  FS3 = 19,   // F19, callee-saved 3
  FS4 = 20,   // F20, callee-saved 4
  FS5 = 21,   // F21, callee-saved 5
  FS6 = 22,   // F22, callee-saved 6
  FS7 = 23,   // F23, callee-saved 7
  FS8 = 24,   // F24, callee-saved 8
  FS9 = 25,   // F25, callee-saved 9
  FS10 = 26,  // F26, callee-saved 10
  FS11 = 27,  // F27, callee-saved 11

  FT8 = 28,   // F28, temporary 8
  FT9 = 29,   // F29, temporary 9
  FT10 = 30,  // F30, temporary 10
  FT11 = 31,  // F31, temporary 11

  kNumberOfFRegisters = 32,
  kNoFRegister = -1,  // Signals an illegal F register.

  FTMP = FT11,  // Reserved for special uses, such as assembler macro instructions.
};

std::ostream& operator<<(std::ostream& os, const FRegister& rhs);

enum VRegister {
  V0 = 0,  // V0, argument 0
  V1 = 1,  // V1, callee-saved 0
  V2 = 2,  // V2, callee-saved 1
  V3 = 3,  // V3, callee-saved 2
  V4 = 4,  // V4, callee-saved 3
  V5 = 5,  // V5, callee-saved 4
  V6 = 6,  // V6, callee-saved 5
  V7 = 7,  // V7, callee-saved 6

  V8 = 8,    // V8, argument 1
  V9 = 9,    // V9, argument 2
  V10 = 10,  // V10, argument 3
  V11 = 11,  // V11, argument 4
  V12 = 12,  // V12, argument 5
  V13 = 13,  // V13, argument 6
  V14 = 14,  // V14, argument 7
  V15 = 15,  // V15, argument 8

  V16 = 16,  // V16, argument 9
  V17 = 17,  // V17, argument 10
  V18 = 18,  // V18, argument 11
  V19 = 19,  // V19, argument 12
  V20 = 20,  // V20, argument 13
  V21 = 21,  // V21, argument 14
  V22 = 22,  // V22, argument 15
  V23 = 23,  // V23, argument 16

  V24 = 24,  // V24, callee-saved 7
  V25 = 25,  // V25, callee-saved 8
  V26 = 26,  // V26, callee-saved 9
  V27 = 27,  // V27, callee-saved 10
  V28 = 28,  // V28, callee-saved 11
  V29 = 29,  // V29, callee-saved 12
  V30 = 30,  // V30, callee-saved 13
  V31 = 31,  // V31, callee-saved 14

  kNumberOfVRegisters = 32,
  kNoVRegister = -1,  // Signals an illegal V register.
};

std::ostream& operator<<(std::ostream& os, const VRegister& rhs);

}  // namespace riscv64
}  // namespace art

#endif  // ART_RUNTIME_ARCH_RISCV64_REGISTERS_RISCV64_H_
