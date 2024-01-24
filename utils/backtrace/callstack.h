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

#ifndef UTILS_BACKTRACE_CALLSTACK_H_
#define UTILS_BACKTRACE_CALLSTACK_H_

#include <stdint.h>
#include <sys/types.h>

#define CALLSTACK_MODULE_PATH "/system/lib64/libutilscallstack.so"
#define CALLSTACK_DEFAULT_SYMB "_ZN7android9CallStackC2Ev"
#define CALLSTACK_CONSTRUCTOR_CI_SYMB "_ZN7android9CallStackC1EPKci"
#define CALLSTACK_UPDATE_SYMB "_ZN7android9CallStack6updateEii"
#define CALLSTACK_DUMP_SYMB "_ZNK7android9CallStack4dumpEiiPKc"

class CallStack {
public:
    CallStack();
    CallStack(const char* logtag, int32_t ignoreDepth);
    void update(int32_t ignoreDepth = 1, pid_t tid = -1);
    void dump(int fd, int indent = 0, const char* prefix = nullptr);
private:
    __attribute__ ((__unused__)) uint8_t mFrameLines[40];
    __attribute__ ((__unused__)) uint8_t reserved[216];
};

#endif // UTILS_BACKTRACE_CALLSTACK_H_
