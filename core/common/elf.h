/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
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

#ifndef CORE_COMMON_ELF_H_
#define CORE_COMMON_ELF_H_

#include <stdint.h>
#include <sys/types.h>

#define EM_NONE     0
#define EM_386      3
#define EM_ARM      40
#define EM_X86_64   62
#define EM_AARCH64  183
#define EM_RISCV    243

class ElfHeader {
public:
    uint8_t  ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
};

#endif  // CORE_COMMON_ELF_H_
