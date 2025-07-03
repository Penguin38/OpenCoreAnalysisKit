/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef CORE_COMMON_TLS_H_
#define CORE_COMMON_TLS_H_

#include <stdint.h>
#include <sys/types.h>

namespace arm64 {

struct tls {
    uint64_t tpidr_el0;
    uint64_t tpidr2_el0;
};

typedef struct elf64_tls {
    struct tls regs;
} Elf64_tls;

} // namespace arm64

#endif  // CORE_COMMON_TLS_H_
