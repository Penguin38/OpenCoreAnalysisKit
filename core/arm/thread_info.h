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
};

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) {}
    ~ThreadInfo() {}

    Register  reg;
};

} // namespace arm

#endif // CORE_ARM_THREAD_INFO_H_
