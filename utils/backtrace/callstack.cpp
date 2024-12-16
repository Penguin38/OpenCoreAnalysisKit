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

#include "logger/log.h"
#include "backtrace/callstack.h"
#include <stdlib.h>
#include <dlfcn.h>

#if defined(__LINUX__) || defined(__MACOS__)
#include <cxxabi.h>
#include <execinfo.h>
#endif

static void *sHandle;

CallStack::CallStack() {
#if defined(__ANDROID__)
    if (!sHandle)
        sHandle = dlopen(CALLSTACK_MODULE_PATH, RTLD_LAZY);

    if (sHandle) {
        void* constructor = dlsym(sHandle, CALLSTACK_DEFAULT_SYMB);
        if (constructor) {
            typedef void (*CallStackDefault)(void *ptr);
            ((CallStackDefault)constructor)((void *)this);
        }
    }
#endif
}

CallStack::CallStack(const char* logtag, int32_t ignoreDepth) {
#if defined(__ANDROID__)
    if (!sHandle)
        sHandle = dlopen(CALLSTACK_MODULE_PATH, RTLD_LAZY);

    if (sHandle) {
        void* constructor_ci = dlsym(sHandle, CALLSTACK_CONSTRUCTOR_CI_SYMB);
        if (constructor_ci) {
            typedef long (*CallStackConstructor)(void *ptr, const char* logtag, int32_t ignoreDepth);
            ((CallStackConstructor)constructor_ci)((void *)this, logtag, ignoreDepth);
        }
    }
#endif
}

void CallStack::update(int32_t ignoreDepth, pid_t tid) {
#if defined(__ANDROID__)
    if (sHandle) {
        void* updateimpl = dlsym(sHandle, CALLSTACK_UPDATE_SYMB);
        if (updateimpl) {
            typedef void (*CallStackUpdate)(void *ptr, int32_t ignoreDepth, pid_t tid);
            ((CallStackUpdate)updateimpl)((void *)this, ignoreDepth, tid);
        }
    }
#endif
}

void CallStack::dump(int fd, int indent, const char* prefix) {
#if defined(__ANDROID__)
    if (sHandle) {
        void* dumpimpl = dlsym(sHandle, CALLSTACK_DUMP_SYMB);
        if (dumpimpl) {
            typedef void (*CallStackDump)(void *ptr, int fd, int indent, const char* prefix);
            (CallStackDump(dumpimpl))((void *)this, fd, indent, prefix);
        }
    }
#else
    int layers, i;
    char **symbols = nullptr;
    void *frames[24];
    memset(frames, 0, sizeof(frames));
    layers = backtrace(frames, 24);
    symbols = backtrace_symbols(frames, layers);
    if (symbols) {
        for (i = 0; i < layers; i++) {
            LOGI("#%02d pc %s\n", i, symbols[i]);
        }
        free(symbols);
    }
#endif
}
