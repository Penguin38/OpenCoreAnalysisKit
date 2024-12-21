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

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) {}
    ThreadInfo(int tid, uint64_t prs) : ThreadApi(tid, prs) {}
    ~ThreadInfo() {}
    void RegisterDump(const char* prefix) { return reg.Dump(prefix); }
    void RegisterSet(const char* command);
    uint64_t RegisterGet(const char* regs);
    Register& GetRegs() { return reg; }
    uint64_t GetFramePC() { return GetRegs().pc; }
    uint64_t GetFrameSP() { return GetRegs().sp; }

    Register  reg;
};

} // namespace arm64

#endif // CORE_ARM64_THREAD_INFO_H_
