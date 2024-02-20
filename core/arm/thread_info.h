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

#ifndef CORE_ARM_THREAD_INFO_H_
#define CORE_ARM_THREAD_INFO_H_

#include "logger/log.h"
#include "api/thread.h"

namespace arm {

class Register {
public:
    uint32_t  r0;
    uint32_t  r1;
    uint32_t  r2;
    uint32_t  r3;
    uint32_t  r4;
    uint32_t  r5;
    uint32_t  r6;
    uint32_t  r7;
    uint32_t  r8;
    uint32_t  r9;
    uint32_t  r10;
    uint32_t  r11;
    uint32_t  r12;
    uint32_t  sp;
    uint32_t  lr;
    uint32_t  pc;
    uint32_t  cpsr;

    void Dump(const char* prefix) {
        LOGI("%sr0  0x%08x  r1  0x%08x  r2  0x%08x  r3  0x%08x  \n", prefix, r0, r1, r2, r3);
        LOGI("%sr4  0x%08x  r5  0x%08x  r6  0x%08x  r7  0x%08x  \n", prefix, r4, r5, r6, r7);
        LOGI("%sr8  0x%08x  r9  0x%08x  r10 0x%08x  r11 0x%08x  \n", prefix, r8, r9, r10, r11);
        LOGI("%sr12 0x%08x  sp  0x%08x  lr  0x%08x  pc  0x%08x  cpsr 0x%08x  \n", prefix, r12, sp, lr, pc, cpsr);
    }
};

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) {}
    ~ThreadInfo() {}
    void RegisterDump(const char* prefix) { return reg.Dump(prefix); }
    Register& GetRegs() { return reg; }

    Register  reg;
};

} // namespace arm

#endif // CORE_ARM_THREAD_INFO_H_
