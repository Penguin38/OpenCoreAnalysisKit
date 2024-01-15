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
#include "command/help.h"
#include "command/command_manager.h"

void Help::printCommandUsage(const char* cmd) {
    Command* command = CommandManager::FindCommand(cmd);
    if (command) {
        return command->usage();
    } else {
        LOGI("Not found command (%s).\n", cmd);
    }
}

void Help::printCommands() {
    int index = 0;
    auto callback = [&index](Command* command) -> bool {
        LOGI("%s\n", command->get().c_str());
        return false;
    };
    CommandManager::ForeachCommand(callback);
}

int Help::main(int argc, char* const argv[]) {
    if (!argc) {
        printCommands();
    } else {
        printCommandUsage(argv[0]);
    }
    return 0;
}
