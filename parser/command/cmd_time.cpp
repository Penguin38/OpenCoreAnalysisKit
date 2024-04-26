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

#include "logger/log.h"
#include "command/cmd_time.h"
#include "command/command_manager.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

int TimeCommand::main(int argc, char* const argv[]) {
    if (!(argc > 1)) return 0;
    auto starttime = system_clock::now();
    CommandManager::Execute(argv[1], argc - 1, &argv[1]);
    duration<double> diff = system_clock::now()- starttime;
    LOGI("\nTotal time: %lf (seconds)\n", diff.count());
    return 0;
}

void TimeCommand::usage() {
    LOGI("Usage: time <COMMAND> [options..]\n");
}
