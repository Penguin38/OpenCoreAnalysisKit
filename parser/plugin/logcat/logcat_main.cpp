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
#include "android.h"
#include "command/command_manager.h"
#include "command/command.h"
#include <string>

class LogcatCommand : public Command {
public:
    LogcatCommand() : Command("logcat") {}
    ~LogcatCommand() {}
    int main(int argc, char* const argv[]);
    void usage();
};

void __attribute__((constructor)) logcat_init(void) {
    CommandManager::PushExtendCommand(new LogcatCommand());
}

void __attribute__((destructor)) logcat_fini(void) {}

int LogcatCommand::main(int argc, char* const argv[]) {
    if (Android::Sdk() >= Android::S) {

    } else {
        LOGI("Not support sdk < %d, please run \"env config --sdk <VERSION>\".\n", Android::S);
    }
    return 0;
}

void LogcatCommand::usage() {
    LOGI("Usage: logcat [option]...\n");
}
