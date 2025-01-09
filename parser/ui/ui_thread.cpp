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
#include "ui/ui_thread.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#if defined(__MACOS__)
#include <pthread.h>
#else
#include <sys/prctl.h>
#endif
#include <chrono>

void UiThread::prepare() {
#if defined(__MACOS__)
    pthread_setname_np("parser:ui");
#else
    prctl(PR_SET_NAME, "parser:worker");
#endif
}

void UiThread::GetCommand(std::string* result) {
    std::unique_lock<std::mutex> lock(cmdlock);
    cond.wait(lock, [this] {
        return ready;
    });
    result->append(cmdline);
}

void UiThread::Wait() {
    std::unique_lock<std::mutex> lock(execlock);
    cond.wait(lock, [this] {
        return !ready;
    });
}

void UiThread::Wake() {
    std::lock_guard<std::mutex> lock(execlock);
    ready = false;
    cond.notify_one();
}

void UiThread::run() {
    prepare();
    shell.Init(parser());
    while (1) {
        {
            std::lock_guard<std::mutex> lock(cmdlock);
            cmdline = shell.HandleCommand();
            ready = true;
        }
        cond.notify_one();
        Wait();
    }
}

void UiThread::runMain(UiThread *ui) {
    ui->run();
}
