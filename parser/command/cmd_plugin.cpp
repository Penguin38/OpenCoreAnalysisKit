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
#include "command/cmd_plugin.h"
#include "command/command_manager.h"
#include <unistd.h>
#include <getopt.h>
#include <dlfcn.h>

int PluginCommand::main(int argc, char* const argv[]) {
    int need_unload = false;
    int need_reload = false;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"unload",  no_argument,       0, 'u'},
        {"reload",  no_argument,       0, 'r'},
    };

    while ((opt = getopt_long(argc, argv, "ur",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'u':
                // need_unload = true;
                break;
            case 'r':
                // need_reload = true;
                break;
        }
    }

    if (optind >= argc) {
        ShowEnv();
        return 0;
    }

    if (need_unload) {
        return UnLoad(argv[optind]);
    } else {
        return Load(argv[optind], need_reload);
    }
    return 0;
}

int PluginCommand::UnLoad(const char* path) {
    auto it = std::remove_if(plugins.begin(), plugins.end(),
        [&](const std::unique_ptr<Plugin>& plugin) {
            return plugin->Path() == path;
        }
    );
    if (it != plugins.end()) {
        Plugin* plugin = it->get();
        Command* command = plugin->Cmd();
        void* handle = plugin->Handle();
        plugins.erase(it);
        if (command) CommandManager::PopExtendCommand(command);
        // if (handle) dlclose(handle);
    }
    return 0;
}

int PluginCommand::Load(const char* path, bool flag) {
    if (flag) UnLoad(path);

    std::unique_ptr<Plugin> plugin = std::make_unique<Plugin>(path, nullptr);
    current_plugin = plugin.get();
    void* handle = dlopen(path, RTLD_NOW);
    // do plugin constructor
    if (plugin->Cmd() && handle) {
        LOGI("Linker env...\n");
        plugin->HookHandle(handle);
        plugins.push_back(std::move(plugin));
        current_plugin = nullptr;
        LOGI("env new command \"%s\"\n", plugins[plugins.size() - 1]->Cmd()->get().c_str());
    } else {
        if (!handle) LOGE("dlopen %s fail!\n", path);
    }

    return 0;
}

void PluginCommand::ShowEnv() {
    for (const auto& it : plugins) {
        LOGI("%016" PRIx64 "  %s  %s\n", (uint64_t)it->Handle(),
                                 it->Path().c_str(),
                                 it->Cmd() ? it->Cmd()->get().c_str() : "");
    }
}

void PluginCommand::usage() {
    LOGI("Usage: plugin <PATH> [Option]\n");
    // LOGI("Option:\n");
    // LOGI("    -u, --unload   remove extend library\n");
    // LOGI("    -r, --reload   reload extend library\n");
    // ENTER();
    LOGI("core-parser> plugin plugin-simple.so\n");
    LOGI("Linker env...\n");
    LOGI("env new command \"simple\"\n");
    LOGI("core-parser> simple\n");
    LOGI("command simple!\n");
}
