/*
 * Copyright (C) 2011 The Android Open Source Project
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

#ifndef ART_RUNTIME_ARCH_X86_REGISTERS_X86_H_
#define ART_RUNTIME_ARCH_X86_REGISTERS_X86_H_

#include <iosfwd>

#include "base/macros.h"

namespace art HIDDEN {
namespace x86 {

enum Register {
  EAX = 0,
  ECX = 1,
  EDX = 2,
  EBX = 3,
  ESP = 4,
  EBP = 5,
  ESI = 6,
  EDI = 7,
  EIP = 8,
  kNumberOfCpuRegisters = 9,
  kFirstByteUnsafeRegister = 4,
  kNoRegister = -1  // Signals an illegal register.
};
std::ostream& operator<<(std::ostream& os, const Register& rhs);
const char* RegisterName(const Register& rhs);

enum XmmRegister {
  XMM0 = 0,
  XMM1 = 1,
  XMM2 = 2,
  XMM3 = 3,
  XMM4 = 4,
  XMM5 = 5,
  XMM6 = 6,
  XMM7 = 7,
  kNumberOfXmmRegisters = 8,
  kNoXmmRegister = -1  // Signals an illegal register.
};
std::ostream& operator<<(std::ostream& os, const XmmRegister& reg);

}  // namespace x86
}  // namespace art

#endif  // ART_RUNTIME_ARCH_X86_REGISTERS_X86_H_
