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

#ifndef PARSER_COMMAND_COMMAND_MANAGER_H_
#define PARSER_COMMAND_COMMAND_MANAGER_H_

#include "command/cmd_plugin.h"
#include "command/command.h"
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

class CommandManager {
public:
    static int Execute(const char* cmd, int argc, char* const argv[]);
    static void PushInlineCommand(Command* command);
    static int PushExtendCommand(Command* command);
    static int PopExtendCommand(Command* command);
    static void ForeachCommand(std::function<bool (Command *)> callback);
    static Command* FindCommand(const char* cmd);
    static void Init();
    ~CommandManager() {
        inline_commands.clear();
        extend_commands.clear();
    }
private:
    void pushInlineCommand(Command* command);
    int pushExtendCommand(Command* command);
    int popExtendCommand(Command* command);
    void foreachInlineCommand(std::function<bool (Command *)> callback);
    void foreachExtendCommand(std::function<bool (Command *)> callback);
    static CommandManager* INSTANCE;
    std::vector<std::unique_ptr<Command>> inline_commands;
    std::vector<std::unique_ptr<Command>> extend_commands;
    PluginCommand *plugin = nullptr;
};

#endif // PARSER_COMMAND_COMMAND_MANAGER_H_
