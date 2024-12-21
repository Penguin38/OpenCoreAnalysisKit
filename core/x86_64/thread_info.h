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

#ifndef CORE_X86_64_THREAD_INFO_H_
#define CORE_X86_64_THREAD_INFO_H_

#include "logger/log.h"
#include "api/thread.h"

namespace x86_64 {

class Register {
public:
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t orig_rax;
    uint64_t rip;
    uint32_t cs;
    uint32_t __cs;
    uint64_t flags;
    uint64_t rsp;
    uint32_t ss;
    uint32_t __ss;
    uint64_t fs_base;
    uint64_t gs_base;
    uint32_t ds;
    uint32_t __ds;
    uint32_t es;
    uint32_t __es;
    uint32_t fs;
    uint32_t __fs;
    uint32_t gs;
    uint32_t __gs;

    void Dump(const char* prefix) {
        LOGI("%srax 0x%016" PRIx64 "  rbx 0x%016" PRIx64 "  rcx 0x%016" PRIx64 "  rdx 0x%016" PRIx64 "  \n", prefix, rax, rbx, rcx, rdx);
        LOGI("%sr8  0x%016" PRIx64 "  r9  0x%016" PRIx64 "  r10 0x%016" PRIx64 "  r11 0x%016" PRIx64 "  \n", prefix, r8, r9, r10, r11);
        LOGI("%sr12 0x%016" PRIx64 "  r13 0x%016" PRIx64 "  r14 0x%016" PRIx64 "  r15 0x%016" PRIx64 "  \n", prefix, r12, r13, r14, r15);
        LOGI("%srdi 0x%016" PRIx64 "  rsi 0x%016" PRIx64 "  \n", prefix, rdi, rsi);
        LOGI("%srbp 0x%016" PRIx64 "  rsp 0x%016" PRIx64 "  rip 0x%016" PRIx64 "  flags 0x%016" PRIx64 "  \n", prefix, rbp, rsp, rip, flags);
        LOGI("%sds 0x%08x  es 0x%08x  fs 0x%08x  gs 0x%08x  cs 0x%08x  ss 0x%08x\n", prefix, ds, es, fs, gs, cs, ss);
    }
};

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) {}
    ThreadInfo(int tid, uint64_t prs) : ThreadApi(tid, prs) {}
    ~ThreadInfo() {}
    void RegisterDump(const char* prefix) { return reg.Dump(prefix); }
    void RegisterSet(const char* command);
    uint64_t RegisterGet(const char* regs);
    Register& GetRegs() { return reg; }
    uint64_t GetFramePC() { return GetRegs().rip; }
    uint64_t GetFrameSP() { return GetRegs().rsp; }

    Register  reg;
};

} // namespace x86_64

#endif // CORE_X86_64_THREAD_INFO_H_
