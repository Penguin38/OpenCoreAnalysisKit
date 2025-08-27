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

#ifndef PARSER_COMMAND_CORE_BACKTRACE_CMD_BACKTRACE_H_
#define PARSER_COMMAND_CORE_BACKTRACE_CMD_BACKTRACE_H_

#include "logger/log.h"
#include "api/thread.h"
#include "command/command.h"
#include <memory>
#include <vector>

class BacktraceCommand : public Command {
public:
    BacktraceCommand() : Command("backtrace", "bt") {}
    ~BacktraceCommand() {}

    class ThreadRecord;

    struct Options : Command::Options {
        bool dump_all;
        bool dump_detail;
        std::vector<uint64_t> dump_fps;
        std::vector<std::unique_ptr<ThreadRecord>> threads;
    };

    int main(int argc, char* const argv[]);
    int prepare(int argc, char* const argv[]);
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
    void DumpJavaStack(void *thread, ThreadApi* api);
    void DumpJavaJniStack(uint32_t *subjni, ThreadApi* api);
    void DumpNativeStack(void *thread, ThreadApi* api);
    static std::string FormatJavaFrame(const char* prefix, uint64_t size);
    static std::string FormatJNINativeFrame(const char* prefix, uint64_t size);
    static std::string FormatNativeFrame(const char* prefix, uint64_t size);
private:
    Options options;
};

#endif // PARSER_COMMAND_CORE_BACKTRACE_CMD_BACKTRACE_H_
