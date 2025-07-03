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

#ifndef CORE_API_THREAD_H_
#define CORE_API_THREAD_H_

#include <vector>
#include <memory>

class NoteBlock;

class ThreadApi {
public:
    struct RegsMap {
        const char* regs;
        int offset;
        int size;
    };

    inline int pid() { return mPid; }
    inline uint64_t prs() { return mPrs; }
    inline NoteBlock* block() { return mBlock; }

    ThreadApi(int tid) : mPid(tid) {}
    ThreadApi(int tid, uint64_t prs) : mPid(tid), mPrs(prs) {}
    void Bind(NoteBlock* block) { mBlock = block; }

    virtual ~ThreadApi() {}
    virtual void RegisterDump(const char* prefix) = 0;
    virtual void FpRegisterDump(const char* prefix) = 0;
    virtual void TlsDump(const char* prefix) = 0;
    virtual void RegisterSet(const char* command) = 0;
    virtual uint64_t RegisterGet(const char* regs) = 0;
    virtual uint64_t GetFramePC() = 0;
    virtual uint64_t GetFrameSP() = 0;
private:
    NoteBlock* mBlock;
    int mPid;
    uint64_t mPrs;
};

#endif // CORE_API_THREAD_H_
