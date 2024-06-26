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

class ThreadApi {
public:
    inline int pid() { return mPid; }

    ThreadApi(int tid) : mPid(tid) {}
    virtual ~ThreadApi() {}
    virtual void RegisterDump(const char* prefix) = 0;
    virtual uint64_t GetFramePC() = 0;
    virtual uint64_t GetFrameSP() = 0;
private:
    int mPid;
};

#endif // CORE_API_THREAD_H_
