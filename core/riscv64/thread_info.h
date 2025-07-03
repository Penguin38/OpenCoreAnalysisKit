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

#ifndef CORE_RISCV64_THREAD_INFO_H_
#define CORE_RISCV64_THREAD_INFO_H_

#include "logger/log.h"
#include "api/thread.h"

namespace riscv64 {

class Register {
public:
    uint64_t pc;
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;

    void Dump(const char* prefix) {
        LOGI("%spc 0x%016" PRIx64 "  ra 0x%016" PRIx64 "  sp 0x%016" PRIx64 "  gp 0x%016" PRIx64 "  \n", prefix, pc, ra, sp, gp);
        LOGI("%stp 0x%016" PRIx64 "  t0 0x%016" PRIx64 "  t1 0x%016" PRIx64 "  t2 0x%016" PRIx64 "  \n", prefix, tp, t0, t1, t2);
        LOGI("%ss0 0x%016" PRIx64 "  s1 0x%016" PRIx64 "  a0 0x%016" PRIx64 "  a1 0x%016" PRIx64 "  \n", prefix, s0, s1, a0, a1);
        LOGI("%sa2 0x%016" PRIx64 "  a3 0x%016" PRIx64 "  a4 0x%016" PRIx64 "  a5 0x%016" PRIx64 "  \n", prefix, a2, a3, a4, a5);
        LOGI("%sa6 0x%016" PRIx64 "  a7 0x%016" PRIx64 "  s2 0x%016" PRIx64 "  s3 0x%016" PRIx64 "  \n", prefix, a6, a7, s2, s3);
        LOGI("%ss4 0x%016" PRIx64 "  s5 0x%016" PRIx64 "  s6 0x%016" PRIx64 "  s7 0x%016" PRIx64 "  \n", prefix, s4, s5, s6, s7);
        LOGI("%ss8 0x%016" PRIx64 "  s9 0x%016" PRIx64 "  s10 0x%016" PRIx64 " s11 0x%016" PRIx64 "  \n", prefix, s8, s9, s10, s11);
        LOGI("%st3 0x%016" PRIx64 "  t4 0x%016" PRIx64 "  t5 0x%016" PRIx64 "  t6 0x%016" PRIx64 "  \n", prefix, t3, t4, t5, t6);
    }
};

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) {}
    ThreadInfo(int tid, uint64_t prs) : ThreadApi(tid, prs) {}
    ~ThreadInfo() {}
    void RegisterDump(const char* prefix) { return reg.Dump(prefix); }
    void FpRegisterDump(const char* prefix) {}
    void TlsDump(const char* prefix) {}
    void RegisterSet(const char* command);
    uint64_t RegisterGet(const char* regs);
    Register& GetRegs() { return reg; }
    uint64_t GetFramePC() { return GetRegs().pc; }
    uint64_t GetFrameSP() { return GetRegs().sp; }

    Register  reg;
};

} // namespace riscv64

#endif // CORE_RISCV64_THREAD_INFO_H_
