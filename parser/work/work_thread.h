/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless reqworkred by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PARSER_WORK_THREAD_H_
#define PARSER_WORK_THREAD_H_

#include <string>
#include <thread>
#include <memory>
#include <iostream>

class WorkThread {
public:
    static constexpr int MAX_ARGC = 16;
    static void Stop(int signal) {}

    WorkThread(std::string c) {
        cmdline = c;
        work = std::make_unique<std::thread>(runMain, this);
    }
    ~WorkThread() { work.reset(); }
    void Join() { work->join(); }

    char* cmd;
    int argc;
    char* argv[MAX_ARGC];
private:
    const char* SPLIT_TOKEN = " \t\n\r";
    static void runMain(WorkThread* thread);
    void run();
    void prepare();

    std::unique_ptr<std::thread> work;
    std::string cmdline;
    std::unique_ptr<char> newline;
};

#endif  // PARSER_WORK_THREAD_H_
