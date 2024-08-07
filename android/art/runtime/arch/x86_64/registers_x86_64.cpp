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

#include "runtime/arch/x86_64/registers_x86_64.h"

#include <ostream>

namespace art HIDDEN {
namespace x86_64 {

static const char* kRegisterNames[] = {
  "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
  "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15", "rip",
};

const char* RegisterName(const Register& rhs) {
  if (rhs >= RAX && rhs <= RIP) {
    return kRegisterNames[rhs];
  }
  return "";
}

std::ostream& operator<<(std::ostream& os, const Register& rhs) {
  if (rhs >= RAX && rhs <= R15) {
    os << kRegisterNames[rhs];
  } else {
    os << "Register[" << static_cast<int>(rhs) << "]";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const FloatRegister& rhs) {
  if (rhs >= XMM0 && rhs <= XMM15) {
    os << "xmm" << static_cast<int>(rhs);
  } else {
    os << "Register[" << static_cast<int>(rhs) << "]";
  }
  return os;
}

}  // namespace x86_64
}  // namespace art
