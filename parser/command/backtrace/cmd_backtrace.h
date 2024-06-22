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

#ifndef PARSER_COMMAND_BACKTRACE_CMD_BACKTRACE_H_
#define PARSER_COMMAND_BACKTRACE_CMD_BACKTRACE_H_

#include "logger/log.h"
#include "api/thread.h"
#include "command/command.h"
#include <memory>
#include <vector>

#if defined(__AOSP_PARSER__)
#include "android.h"
#include "runtime/thread.h"
#endif

class BacktraceCommand : public Command {
public:
    BacktraceCommand() : Command("backtrace", "bt") {}
    ~BacktraceCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) {
#if defined(__AOSP_PARSER__)
        Android::Prepare();
        Android::OatPrepare();
#endif
        return true;
    }
    void usage();
    void addThread(int pid);
    void addThread(int pid, int type);
    void addThread(int pid, int type, void* thread);

    class ThreadRecord {
    public:
        static constexpr int TYPE_NATIVE = 1 << 0;
        static constexpr int TYPE_JVM = 1 << 1;
        ThreadRecord(ThreadApi* p) {
            type = TYPE_NATIVE;
            api = p;
            if (api) pid = api->pid();
            thread = nullptr;
        }
        ThreadRecord(int p) {
            type = TYPE_NATIVE;
            pid = p;
            api = nullptr;
            thread = nullptr;
        }
        ThreadRecord(ThreadApi* p, int t) {
            type = t;
            api = p;
            if (api) pid = api->pid();
            thread = nullptr;
        }
        ThreadRecord(int p, int t) {
            type = t;
            pid = p;
            api = nullptr;
            thread = nullptr;
        }
        int type;
        int pid;
        ThreadApi* api;
        void* thread;
    };

    ThreadRecord* findRecord(int pid);
    void DumpTrace();
    void DumpJavaStack(void *thread);
    void DumpNativeStack(void *thread, ThreadApi* api);
    static std::string FormatJavaFrame(const char* prefix, uint64_t size);
    static std::string FormatNativeFrame(const char* prefix, uint64_t size);
private:
    bool dump_all = false;
    std::vector<std::unique_ptr<ThreadRecord>> threads;
};

#endif // PARSER_COMMAND_BACKTRACE_CMD_BACKTRACE_H_
