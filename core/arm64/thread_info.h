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

#ifndef CORE_ARM64_THREAD_INFO_H_
#define CORE_ARM64_THREAD_INFO_H_

#include "logger/log.h"
#include "api/thread.h"
#include <string.h>

namespace arm64 {

class Register {
public:
    uint64_t  x0;
    uint64_t  x1;
    uint64_t  x2;
    uint64_t  x3;
    uint64_t  x4;
    uint64_t  x5;
    uint64_t  x6;
    uint64_t  x7;
    uint64_t  x8;
    uint64_t  x9;
    uint64_t  x10;
    uint64_t  x11;
    uint64_t  x12;
    uint64_t  x13;
    uint64_t  x14;
    uint64_t  x15;
    uint64_t  x16;
    uint64_t  x17;
    uint64_t  x18;
    uint64_t  x19;
    uint64_t  x20;
    uint64_t  x21;
    uint64_t  x22;
    uint64_t  x23;
    uint64_t  x24;
    uint64_t  x25;
    uint64_t  x26;
    uint64_t  x27;
    uint64_t  x28;
    uint64_t  fp;
    uint64_t  lr;
    uint64_t  sp;
    uint64_t  pc;
    uint64_t  pstate;

    void Dump(const char* prefix) {
        LOGI("%sx0  0x%016" PRIx64 "  x1  0x%016" PRIx64 "  x2  0x%016" PRIx64 "  x3  0x%016" PRIx64 "  \n", prefix, x0, x1, x2, x3);
        LOGI("%sx4  0x%016" PRIx64 "  x5  0x%016" PRIx64 "  x6  0x%016" PRIx64 "  x7  0x%016" PRIx64 "  \n", prefix, x4, x5, x6, x7);
        LOGI("%sx8  0x%016" PRIx64 "  x9  0x%016" PRIx64 "  x10 0x%016" PRIx64 "  x11 0x%016" PRIx64 "  \n", prefix, x8, x9, x10, x11);
        LOGI("%sx12 0x%016" PRIx64 "  x13 0x%016" PRIx64 "  x14 0x%016" PRIx64 "  x15 0x%016" PRIx64 "  \n", prefix, x12, x13, x14, x15);
        LOGI("%sx16 0x%016" PRIx64 "  x17 0x%016" PRIx64 "  x18 0x%016" PRIx64 "  x19 0x%016" PRIx64 "  \n", prefix, x16, x17, x18, x19);
        LOGI("%sx20 0x%016" PRIx64 "  x21 0x%016" PRIx64 "  x22 0x%016" PRIx64 "  x23 0x%016" PRIx64 "  \n", prefix, x20, x21, x22, x23);
        LOGI("%sx24 0x%016" PRIx64 "  x25 0x%016" PRIx64 "  x26 0x%016" PRIx64 "  x27 0x%016" PRIx64 "  \n", prefix, x24, x25, x26, x27);
        LOGI("%sx28 0x%016" PRIx64 "  fp  0x%016" PRIx64 "  \n", prefix, x28, fp);
        LOGI("%slr  0x%016" PRIx64 "  sp  0x%016" PRIx64 "  pc  0x%016" PRIx64 "  pst 0x%016" PRIx64 "  \n", prefix, lr, sp, pc, pstate);
    }
};

class FpRegister {
public:
    uint64_t v0_lo;
    uint64_t v0_hi;
    uint64_t v1_lo;
    uint64_t v1_hi;
    uint64_t v2_lo;
    uint64_t v2_hi;
    uint64_t v3_lo;
    uint64_t v3_hi;
    uint64_t v4_lo;
    uint64_t v4_hi;
    uint64_t v5_lo;
    uint64_t v5_hi;
    uint64_t v6_lo;
    uint64_t v6_hi;
    uint64_t v7_lo;
    uint64_t v7_hi;
    uint64_t v8_lo;
    uint64_t v8_hi;
    uint64_t v9_lo;
    uint64_t v9_hi;
    uint64_t v10_lo;
    uint64_t v10_hi;
    uint64_t v11_lo;
    uint64_t v11_hi;
    uint64_t v12_lo;
    uint64_t v12_hi;
    uint64_t v13_lo;
    uint64_t v13_hi;
    uint64_t v14_lo;
    uint64_t v14_hi;
    uint64_t v15_lo;
    uint64_t v15_hi;
    uint64_t v16_lo;
    uint64_t v16_hi;
    uint64_t v17_lo;
    uint64_t v17_hi;
    uint64_t v18_lo;
    uint64_t v18_hi;
    uint64_t v19_lo;
    uint64_t v19_hi;
    uint64_t v20_lo;
    uint64_t v20_hi;
    uint64_t v21_lo;
    uint64_t v21_hi;
    uint64_t v22_lo;
    uint64_t v22_hi;
    uint64_t v23_lo;
    uint64_t v23_hi;
    uint64_t v24_lo;
    uint64_t v24_hi;
    uint64_t v25_lo;
    uint64_t v25_hi;
    uint64_t v26_lo;
    uint64_t v26_hi;
    uint64_t v27_lo;
    uint64_t v27_hi;
    uint64_t v28_lo;
    uint64_t v28_hi;
    uint64_t v29_lo;
    uint64_t v29_hi;
    uint64_t v30_lo;
    uint64_t v30_hi;
    uint64_t v31_lo;
    uint64_t v31_hi;
    uint32_t fpsr;
    uint32_t fpcr;
    uint32_t __reserved[2];

    void Dump(const char* prefix) {
        LOGI("%sv0  0x%016" PRIx64 "%016" PRIx64 "  v1  0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v0_hi, v0_lo, v1_hi, v1_lo);
        LOGI("%sv2  0x%016" PRIx64 "%016" PRIx64 "  v3  0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v2_hi, v2_lo, v3_hi, v3_lo);
        LOGI("%sv4  0x%016" PRIx64 "%016" PRIx64 "  v5  0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v4_hi, v4_lo, v5_hi, v5_lo);
        LOGI("%sv6  0x%016" PRIx64 "%016" PRIx64 "  v7  0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v6_hi, v6_lo, v7_hi, v7_lo);
        LOGI("%sv8  0x%016" PRIx64 "%016" PRIx64 "  v9  0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v8_hi, v8_lo, v9_hi, v9_lo);
        LOGI("%sv10 0x%016" PRIx64 "%016" PRIx64 "  v11 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v10_hi, v10_lo, v11_hi, v11_lo);
        LOGI("%sv12 0x%016" PRIx64 "%016" PRIx64 "  v13 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v12_hi, v12_lo, v13_hi, v13_lo);
        LOGI("%sv14 0x%016" PRIx64 "%016" PRIx64 "  v15 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v14_hi, v14_lo, v15_hi, v15_lo);
        LOGI("%sv16 0x%016" PRIx64 "%016" PRIx64 "  v17 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v16_hi, v16_lo, v17_hi, v17_lo);
        LOGI("%sv18 0x%016" PRIx64 "%016" PRIx64 "  v19 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v18_hi, v18_lo, v19_hi, v19_lo);
        LOGI("%sv20 0x%016" PRIx64 "%016" PRIx64 "  v21 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v20_hi, v20_lo, v21_hi, v21_lo);
        LOGI("%sv22 0x%016" PRIx64 "%016" PRIx64 "  v23 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v22_hi, v22_lo, v23_hi, v23_lo);
        LOGI("%sv24 0x%016" PRIx64 "%016" PRIx64 "  v25 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v24_hi, v24_lo, v25_hi, v25_lo);
        LOGI("%sv26 0x%016" PRIx64 "%016" PRIx64 "  v27 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v26_hi, v26_lo, v27_hi, v27_lo);
        LOGI("%sv28 0x%016" PRIx64 "%016" PRIx64 "  v29 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v28_hi, v28_lo, v29_hi, v29_lo);
        LOGI("%sv30 0x%016" PRIx64 "%016" PRIx64 "  v31 0x%016" PRIx64 "%016" PRIx64 "  \n", prefix, v30_hi, v30_lo, v31_hi, v31_lo);
        LOGI("%sfpsr 0x%08x  \n", prefix, fpsr);
        LOGI("%sfpcr 0x%08x  \n", prefix, fpcr);
    }
};

class Tls {
public:
    uint64_t tpidr_el0;
    uint64_t tpidr2_el0;

    void Dump(const char* prefix) {
        LOGI("%stpidr_el0 0x%016" PRIx64 "  \n", prefix, tpidr_el0);
    }
};

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) { clear(); }
    ThreadInfo(int tid, uint64_t prs) : ThreadApi(tid, prs) { clear(); }
    ~ThreadInfo() {}
    inline void clear() {
        memset(&reg, 0x0, sizeof(Register));
        memset(&fpr, 0x0, sizeof(FpRegister));
        memset(&tls, 0x0, sizeof(Tls));
    }
    void RegisterDump(const char* prefix) { return reg.Dump(prefix); }
    void FpRegisterDump(const char* prefix) { return fpr.Dump(prefix); }
    void TlsDump(const char* prefix) { return tls.Dump(prefix); }
    void RegisterSet(const char* command);
    uint64_t RegisterGet(const char* regs);
    Register& GetRegs() { return reg; }
    uint64_t GetFramePC() { return GetRegs().pc; }
    uint64_t GetFrameSP() { return GetRegs().sp; }

    Register reg;
    FpRegister fpr;
    Tls tls;
};

} // namespace arm64

#endif // CORE_ARM64_THREAD_INFO_H_
