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
                need_unload = true;
                break;
            case 'r':
                need_reload = true;
                break;
        }
    }

    if (optind >= argc) {
        ShowEnv();
        return 0;
    }

    LOGI("Linker env...\n");
    if (need_unload) {
        return UnLoad(argv[optind]);
    } else {
        return Load(argv[optind], need_reload);
    }
    return 0;
}

int PluginCommand::UnLoad(const char* path) {
    std::vector<std::unique_ptr<Plugin>>::iterator iter;
    for (iter = plugins.begin(); iter != plugins.end(); iter++) {
        if (iter->get()->Path() == path) {
            Plugin* plugin = iter->get();
            if (plugin->Cmd()) CommandManager::PopExtendCommand(plugin->Cmd());
            if (plugin->Handle()) dlclose(plugin->Handle());
            plugins.erase(iter);
            return 1;
        }
    }
    return 0;
}

int PluginCommand::Load(const char* path, bool flag) {
    if (flag) UnLoad(path);

    std::unique_ptr<Plugin> plugin = std::make_unique<Plugin>(path, nullptr);
    plugins.push_back(std::move(plugin));

    void* handle = dlopen(path, RTLD_NOW);
    // do plugin constructor
    if (plugins[plugins.size() - 1]->Cmd()) {
        plugins[plugins.size() - 1]->HookHandle(handle);
        LOGI("env new command \"%s\"\n", plugins[plugins.size() - 1]->Cmd()->get().c_str());
    } else {
        if (!handle) LOGE("ERROR: dlopen %s fail!\n", path);
        plugins.pop_back();
    }

    return 0;
}

void PluginCommand::ShowEnv() {
    for (const auto& it : plugins) {
        LOGI("%016lx  %s  %s\n", (uint64_t)it->Handle(),
                                 it->Path().c_str(),
                                 it->Cmd() ? it->Cmd()->get().c_str() : "");
    }
}

void PluginCommand::usage() {
    LOGI("Usage: plugin <PATH> [Option]");
    LOGI("Option:\n");
    LOGI("    --unload|-u: remove extend library.\n");
    LOGI("    --reload|-r: reload extend library.\n");
}
