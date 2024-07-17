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
#include "command/command.h"
#include "command/command_manager.h"

class SimpleCommand : public Command {
public:
    SimpleCommand() : Command("simple") {}
    ~SimpleCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) {
        return true;
    }
    void usage();
};

void __attribute__((constructor)) simple_init(void) {
    CommandManager::PushExtendCommand(new SimpleCommand());
}

void __attribute__((destructor)) simple_fini(void) {}

int SimpleCommand::main(int argc, char* const argv[]) {
    LOGI("command simple!\n");
    return 0;
}

void SimpleCommand::usage() {
    LOGI("Usage: simple\n");
}
