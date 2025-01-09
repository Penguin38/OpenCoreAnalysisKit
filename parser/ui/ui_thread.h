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

#ifndef PARSER_UI_THREAD_H_
#define PARSER_UI_THREAD_H_

#include "ui/mini_shell.h"
#include <mutex>
#include <string>
#include <thread>
#include <memory>
#include <condition_variable>
#include <iostream>

class UiThread {
public:
    UiThread() {
        ui = std::make_unique<std::thread>(runMain, this);
    }
    ~UiThread() { ui.reset(); }
    void Join() { ui->join(); }
    void GetCommand(std::string* result);
    void Wake();
private:
    static void runMain(UiThread* thread);
    void Wait();
    void run();
    void prepare();
    inline std::string& cmd() { return cmdline; }
    inline char* parser() { return name; }

    std::unique_ptr<std::thread> ui;
    char name[16] = "core-parser";
    std::string cmdline;
    std::mutex cmdlock;
    std::mutex execlock;
    std::condition_variable cond;
    bool ready = false;
    MiniShell shell;
};

#endif  // PARSER_UI_THREAD_H_
