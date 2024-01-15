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
};

class ThreadInfo : public ThreadApi {
public:
    ThreadInfo(int tid) : ThreadApi(tid) {}
    ~ThreadInfo() {}

    Register  reg;
};

} // namespace arm64

#endif // CORE_ARM64_THREAD_INFO_H_
