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

#include "runtime/arch/riscv64/registers_riscv64.h"

#include <ostream>

namespace art HIDDEN {
namespace riscv64 {

static const char* kXRegisterNames[] = {"zero", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                                        "fp",   "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                                        "a6",   "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                                        "s8",   "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

static const char* kFRegisterNames[] = {"ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
                                        "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
                                        "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
                                        "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"};

static const char* kVRegisterNames[] = {"v0",  "v1",  "v2",  "v3",  "v4",  "v5",  "v6",  "v7",
                                        "v8",  "v9",  "v10", "v11", "v12", "v13", "v14", "v15",
                                        "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
                                        "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31"};

std::ostream& operator<<(std::ostream& os, const XRegister& rhs) {
  if (rhs >= Zero && rhs < kNumberOfXRegisters) {
    os << kXRegisterNames[rhs];
  } else {
    os << "XRegister[" << static_cast<int>(rhs) << "]";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const FRegister& rhs) {
  if (rhs >= FT0 && rhs < kNumberOfFRegisters) {
    os << kFRegisterNames[rhs];
  } else {
    os << "FRegister[" << static_cast<int>(rhs) << "]";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const VRegister& rhs) {
  if (rhs >= V0 && rhs < kNumberOfVRegisters) {
    os << kVRegisterNames[rhs];
  } else {
    os << "VRegister[" << static_cast<int>(rhs) << "]";
  }
  return os;
}

}  // namespace riscv64
}  // namespace art
