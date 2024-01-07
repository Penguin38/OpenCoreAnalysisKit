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

#ifndef CORE_COMMON_PRSTATUS_H_
#define CORE_COMMON_PRSTATUS_H_

#include <stdint.h>
#include <sys/types.h>

namespace arm64 {

struct pt_regs {
    uint64_t  regs[31];
    uint64_t  sp;
    uint64_t  pc;
    uint64_t  pstate;
} __attribute__((aligned(1)));

typedef struct elf64_prstatus {
    uint32_t             pr_si_signo;
    uint32_t             pr_si_code;
    uint32_t             pr_si_errno;
    uint16_t             pr_cursig;
    uint64_t             pr_sigpend;
    uint64_t             pr_sighold;
    uint32_t             pr_pid;
    uint32_t             pr_ppid;
    uint32_t             pr_pgrp;
    uint32_t             pd_sid;
    uint64_t             pr_utime[2];
    uint64_t             pr_stime[2];
    uint64_t             pr_cutime[2];
    uint64_t             pr_cstime[2];
    struct pt_regs       pr_reg;
    uint32_t             pr_fpvalid;
} Elf64_prstatus;

} // namespace arm64

namespace arm {

struct pt_regs {
    uint32_t  regs[13];
    uint32_t  sp;
    uint32_t  lr;
    uint32_t  pc;
    uint32_t  cpsr;
} __attribute__((aligned(1)));

typedef struct elf32_prstatus {
    uint32_t             pr_si_signo;
    uint32_t             pr_si_code;
    uint32_t             pr_si_errno;
    uint16_t             pr_cursig;
    uint16_t             __padding1;
    uint32_t             pr_sigpend;
    uint32_t             pr_sighold;
    uint32_t             pr_pid;
    uint32_t             pr_ppid;
    uint32_t             pr_pgrp;
    uint32_t             pd_sid;
    uint64_t             pr_utime;
    uint64_t             pr_stime;
    uint64_t             pr_cutime;
    uint64_t             pr_cstime;
    struct pt_regs       pr_reg;
    uint32_t             pr_fpvalid;
    uint32_t             __padding2;
} __attribute__((packed, aligned(1))) Elf32_prstatus;

} // namespace arm

#endif  // CORE_COMMON_PRSTATUS_H_
