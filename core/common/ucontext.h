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

#ifndef CORE_COMMON_UCONTEXT_H_
#define CORE_COMMON_UCONTEXT_H_

#include "common/prstatus.h"

namespace lp64 {

struct stack_t {
    uint64_t ss_sp;
    int32_t ss_flags;
    uint64_t ss_size;
};

} // namespace lp64

namespace lp32 {

struct stack_t {
    uint32_t ss_sp;
    int32_t ss_flags;
    uint32_t ss_size;
};

} // namespace lp32

namespace arm64 {

struct mcontext {
    uint64_t fault_address;
    uint64_t regs[31];
    uint64_t sp;
    uint64_t pc;
    uint64_t pstate;
    uint8_t  __reserved[4096] __attribute__((__aligned__(16)));
};

struct ucontext {
    uint64_t uc_flags;
    uint64_t uc_link;
    struct lp64::stack_t  uc_stack;
    uint64_t uc_sigmask;
    uint8_t  __reserved[120];
    struct mcontext uc_mcontext;
};

} // namespace arm64

namespace arm {

struct mcontext {
    uint32_t trap_no;
    uint32_t error_code;
    uint32_t oldmask;
    uint32_t arm_r0;
    uint32_t arm_r1;
    uint32_t arm_r2;
    uint32_t arm_r3;
    uint32_t arm_r4;
    uint32_t arm_r5;
    uint32_t arm_r6;
    uint32_t arm_r7;
    uint32_t arm_r8;
    uint32_t arm_r9;
    uint32_t arm_r10;
    uint32_t arm_fp;
    uint32_t arm_ip;
    uint32_t arm_sp;
    uint32_t arm_lr;
    uint32_t arm_pc;
    uint32_t arm_cpsr;
    uint32_t fault_address;
};

struct ucontext {
    uint32_t uc_flags;
    uint32_t uc_link;
    struct lp32::stack_t uc_stack;
    struct mcontext uc_mcontext;
    union {
        struct {
            uint32_t uc_sigmask;
            uint32_t __padding_rt_sigset;
        };
        uint64_t uc_sigmask64;
    };
    uint8_t __padding[120];
    uint32_t uc_regspace[128] __attribute__((__aligned__(8)));
};

} // namespace arm

namespace x86_64 {

struct mcontext {
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t di;
    uint64_t si;
    uint64_t bp;
    uint64_t bx;
    uint64_t dx;
    uint64_t ax;
    uint64_t cx;
    uint64_t sp;
    uint64_t ip;
    uint64_t flags;
    uint16_t cs;
    uint16_t gs;
    uint16_t fs;
    uint16_t ss;
    uint64_t err;
    uint64_t trapno;
    uint64_t oldmask;
    uint64_t cr2;
    uint64_t fpstate;
    uint64_t reserved1[8];
};

struct ucontext {
    uint64_t uc_flags;
    uint64_t uc_link;
    struct lp64::stack_t uc_stack;
    struct mcontext uc_mcontext;
    uint64_t uc_sigmask;
};

} // namespace x86_64

namespace x86 {

struct mcontext {
    uint16_t gs, __gsh;
    uint16_t fs, __fsh;
    uint16_t es, __esh;
    uint16_t ds, __dsh;
    uint32_t di;
    uint32_t si;
    uint32_t bp;
    uint32_t sp;
    uint32_t bx;
    uint32_t dx;
    uint32_t cx;
    uint32_t ax;
    uint32_t trapno;
    uint32_t err;
    uint32_t ip;
    uint16_t cs, __csh;
    uint32_t flags;
    uint32_t sp_at_signal;
    uint16_t ss, __ssh;
    uint32_t fpstate;
    uint32_t oldmask;
    uint32_t cr2;
};

struct ucontext {
    uint32_t uc_flags;
    uint32_t uc_link;
    struct lp32::stack_t uc_stack;
    struct mcontext uc_mcontext;
    uint32_t uc_sigmask;
};

} // namespace x86

namespace riscv64 {

struct __riscv_v_ext_state {
    uint64_t vstart;
    uint64_t vl;
    uint64_t vtype;
    uint64_t vcsr;
    uint64_t vlenb;
    uint64_t datap;
};

struct __sc_riscv_v_state {
    struct __riscv_v_ext_state v_state;
} __attribute__((aligned(16)));


struct __riscv_f_ext_state {
    uint32_t f[32];
    uint32_t fcsr;
};

struct __riscv_d_ext_state {
    uint64_t f[32];
    uint32_t fcsr;
};

struct __riscv_q_ext_state {
    uint64_t f[64] __attribute__((aligned(16)));
    uint32_t fcsr;
    uint32_t reserved[3];
};

union __riscv_fp_state {
    struct __riscv_f_ext_state f;
    struct __riscv_d_ext_state d;
    struct __riscv_q_ext_state q;
};

struct __riscv_ctx_hdr {
    uint32_t magic;
    uint32_t size;
};

struct __riscv_extra_ext_header {
    uint32_t __padding[129] __attribute__((aligned(16)));
    uint32_t reserved;
    struct __riscv_ctx_hdr hdr;
};

struct mcontext {
    struct riscv64::pt_regs sc_regs;
    union {
        union __riscv_fp_state sc_fpregs;
        struct __riscv_extra_ext_header sc_extdesc;
    };
};

struct ucontext {
    uint64_t uc_flags;
    uint64_t uc_link;
    struct lp64::stack_t  uc_stack;
    uint64_t uc_sigmask;
    uint8_t  __reserved[120];
    struct mcontext uc_mcontext;
};

} // namespace riscv64

#endif  // CORE_COMMON_UCONTEXT_H_
