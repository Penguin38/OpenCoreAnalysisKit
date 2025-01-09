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

#ifndef CORE_X86_THREAD_INFO_H_
#define CORE_X86_THREAD_INFO_H_

#include "logger/log.h"
#include "api/thread.h"

namespace x86 {

class Register {
public:
    uint32_t ebx, ecx, edx, esi, edi, ebp, eax;
    uint16_t ds, __ds, es, __es;
    uint16_t fs, __fs, gs, __gs;
    uint32_t orig_eax, eip;
    uint16_t cs, __cs;
    uint32_t eflags, esp;
    uint16_t ss, __ss;

    void Dump(const char* prefix) {
        LOGI("%seax 0x%08x  ebx 0x%08x  ecx 0x%08x  edx 0x%08x  \n", prefix, eax, ebx, ecx, edx);
        LOGI("%sedi 0x%08x  esi 0x%08x  \n", prefix, edi, esi);
        LOGI("%sebp 0x%08x  esp 0x%08x  eip 0x%08x  eflags 0x%08x  \n", prefix, ebp, esp, eip, eflags);
        LOGI("%sds 0x%04x  es 0x%04x  fs 0x%04x  gs 0x%04x  cs 0x%04x  ss 0x%04x\n", prefix, ds, es, fs, gs, cs, ss);
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
    uint64_t GetFramePC() { return GetRegs().eip; }
    uint64_t GetFrameSP() { return GetRegs().esp; }

    Register  reg;
};

} // namespace x86

#endif // CORE_X86_THREAD_INFO_H_
