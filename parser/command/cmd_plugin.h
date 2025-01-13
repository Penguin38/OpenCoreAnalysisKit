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

#ifndef PARSER_COMMAND_CMD_PLUGIN_H_
#define PARSER_COMMAND_CMD_PLUGIN_H_

#include "command/command.h"
#include <string>
#include <memory>
#include <vector>

class PluginCommand : public Command {
public:
    static constexpr int UNLOAD = 1;
    static constexpr int RELOAD = 2;

    class Plugin {
    public:
        Plugin(const char* p, void *h) {
            if (p) path = p;
            handle = h;
            command = nullptr;
        }
        inline std::string& Path() { return path; }
        inline void* Handle() { return handle; }
        inline Command* Cmd() { return command; }
        void HookHandle(void* h) { handle = h; }
        void HookCommand(Command* cmd) { command = cmd; }
    private:
        std::string path;
        void* handle;
        Command* command;
    };

    PluginCommand() : Command("plugin") {}
    ~PluginCommand() {}
    int main(int argc, char* const argv[]);

    int UnLoad(const char* path);
    int Load(const char* path, bool flag);
    void usage();
    void ShowEnv();
    std::vector<std::unique_ptr<Plugin>>& GetPlugins() { return plugins; }
    Plugin* GetCurrent() { return current_plugin; }
private:
    std::vector<std::unique_ptr<Plugin>> plugins;
    Plugin* current_plugin = nullptr;
};

#endif // PARSER_COMMAND_CMD_PLUGIN_H_
