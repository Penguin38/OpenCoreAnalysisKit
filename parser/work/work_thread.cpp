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

#include "work/work_thread.h"
#include "command/command_manager.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/prctl.h>

void WorkThread::prepare() {
    prctl(PR_SET_NAME, "parser:worker");
    std::unique_ptr<char> line(strdup(cmdline.c_str()));
    char* token = strtok(line.get(), SPLIT_TOKEN);
    while (token != nullptr) {
        if (!argc) {
            cmd = token;
        }

        argv[argc] = token;
        ++argc;
        if (argc > MAX_ARGC - 1)
            break;
        token = strtok(nullptr, SPLIT_TOKEN);
    }
    newline = std::move(line);
}

void WorkThread::run() {
    prepare();
    int optind_backup = optind;
    optind = 0; // reset
    CommandManager::Execute(cmd, argc, argv);
    optind = optind_backup;
}

void WorkThread::runMain(WorkThread* thread) {
    thread->run();
}
