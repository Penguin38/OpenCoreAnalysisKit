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

/*
 * Copyright 2015, VIXL authors. All rights reserved.
 * vixl/src/aarch64/constants-aarch64.h
 */

#ifndef CORE_ARM64_INST_CONSTANT_H_
#define CORE_ARM64_INST_CONSTANT_H_

namespace arm64 {

// Generic fields.
enum GenericInstrField {
  SixtyFourBits        = 0x80000000,
  ThirtyTwoBits        = 0x00000000,

  FPTypeMask           = 0x00C00000,
  FP16                 = 0x00C00000,
  FP32                 = 0x00000000,
  FP64                 = 0x00400000
};

// Move wide immediate.
enum MoveWideImmediateOp {
  MoveWideImmediateFixed = 0x12800000,
  MoveWideImmediateFMask = 0x1F800000,
  MoveWideImmediateMask  = 0xFF800000,
  MOVN                   = 0x00000000,
  MOVZ                   = 0x40000000,
  MOVK                   = 0x60000000,
  MOVN_w                 = MoveWideImmediateFixed | MOVN,
  MOVN_x                 = MoveWideImmediateFixed | MOVN | SixtyFourBits,
  MOVZ_w                 = MoveWideImmediateFixed | MOVZ,
  MOVZ_x                 = MoveWideImmediateFixed | MOVZ | SixtyFourBits,
  MOVK_w                 = MoveWideImmediateFixed | MOVK,
  MOVK_x                 = MoveWideImmediateFixed | MOVK | SixtyFourBits
};

enum SystemPAuthOp {
  SystemPAuthFixed = 0xD503211F,
  SystemPAuthFMask = 0xFFFFFD1F,
  SystemPAuthMask  = 0xFFFFFFFF,
  PACIA1716 = SystemPAuthFixed | 0x00000100,
  PACIB1716 = SystemPAuthFixed | 0x00000140,
  AUTIA1716 = SystemPAuthFixed | 0x00000180,
  AUTIB1716 = SystemPAuthFixed | 0x000001C0,
  PACIAZ    = SystemPAuthFixed | 0x00000300,
  PACIASP   = SystemPAuthFixed | 0x00000320,
  PACIBZ    = SystemPAuthFixed | 0x00000340,
  PACIBSP   = SystemPAuthFixed | 0x00000360,
  AUTIAZ    = SystemPAuthFixed | 0x00000380,
  AUTIASP   = SystemPAuthFixed | 0x000003A0,
  AUTIBZ    = SystemPAuthFixed | 0x000003C0,
  AUTIBSP   = SystemPAuthFixed | 0x000003E0,

  // XPACLRI has the same fixed mask as System Hints and needs to be handled
  // differently.
  XPACLRI   = 0xD50320FF
};

} // namespace arm64

#endif // CORE_ARM64_INST_CONSTANT_H_
