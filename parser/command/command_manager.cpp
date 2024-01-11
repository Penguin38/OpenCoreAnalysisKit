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

#include "command/command_manager.h"

CommandManager* CommandManager::INSTANCE;

int CommandManager::Execute(const char* cmd, int argc, char* const argv[]) {
    return INSTANCE->execute(cmd, argc, argv);
}

int CommandManager::execute(const char* cmd, int argc, char* const argv[]) {
    for (const auto& command : inline_commands) {
        if (command->get() == cmd) {
            return command->main(argc, argv);
        }
    }

    for (const auto& command : extend_commands) {
        if (command->get() == cmd) {
            return command->main(argc, argv);
        }
    }

    std::cout << "Not found command." << std::endl;
    return 0;
}

void CommandManager::PushInlineCommand(Command* command) {
    INSTANCE->pushInlineCommand(command);
}

void CommandManager::pushInlineCommand(Command* command) {
    std::unique_ptr<Command> ref(command);
    inline_commands.push_back(std::move(ref));
}

void CommandManager::PushExtendCommand(Command* command) {
    INSTANCE->pushExtendCommand(command);
}

void CommandManager::pushExtendCommand(Command* command) {
    std::unique_ptr<Command> ref(command);
    extend_commands.push_back(std::move(ref));
}
