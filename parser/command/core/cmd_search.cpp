/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#include "command/core/cmd_search.h"
#include "command/command_manager.h"
#include "logger/log.h"
#include "base/utils.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <sstream>

int CoreSearchCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !(argc > 1))
        return Command::FINISH;

    options.flags = 0x0;
    options.in_stack = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"stack",  no_argument,       0,  's'},
        {"read",   no_argument,       0,  'r'},
        {"write",  no_argument,       0,  'w'},
        {"exec",   no_argument,       0,  'x'},
        {0,        0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "srwx",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 's':
                options.in_stack = true;
                break;
            case 'r':
                options.flags |= Block::FLAG_R;
                break;
            case 'w':
                options.flags |= Block::FLAG_W;
                break;
            case 'x':
                options.flags |= Block::FLAG_X;
                break;
        }
    }
    options.optind = optind;

    if (!options.flags) {
        options.flags |= Block::FLAG_R;
        options.flags |= Block::FLAG_W;
        options.flags |= Block::FLAG_X;
    }

    if (options.optind >= argc) {
        usage();
        return Command::FINISH;
    }

    return Command::ONCHLD;
}

int CoreSearchCommand::main(int argc, char* const argv[]) {
    if (options.in_stack)
        CoreSearchCommand::WalkStack(argv[options.optind]);
    else
        CoreSearchCommand::WalkLoadBlock(argv[options.optind], options.flags);
    return 0;
}

void CoreSearchCommand::WalkStack(const char* value) {
    auto callback = [&](ThreadApi *api) -> bool {
        LOGI("Thread(\"" ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "\")\n", api->pid());
        LoadBlock *block = CoreApi::FindLoadBlock(api->GetFrameSP(), false);
        if (block && block->isValid()) {
            int argc = 7;
            std::string bs = Utils::ToHex(block->vaddr());
            std::string es = Utils::ToHex(block->vaddr() + block->memsz());

            char* argv[8] = {
                const_cast<char*>("rd"),
                const_cast<char*>(bs.c_str()),
                const_cast<char*>("-e"),
                const_cast<char*>(es.c_str()),
                const_cast<char*>("|"),
                const_cast<char*>("grep"),
                const_cast<char*>(value),
                nullptr};
            CommandManager::Execute(argv[0], argc, argv);
        }
        return false;
    };
    CoreApi::ForeachThread(callback);
}

void CoreSearchCommand::WalkLoadBlock(const char* value, int flags) {
    auto callback = [&](LoadBlock *block) -> bool {
        if (!(flags & block->flags()))
            return false;

        if (block->isValid()) {
            int argc = 7;
            std::string bs = Utils::ToHex(block->vaddr());
            std::string es = Utils::ToHex(block->vaddr() + block->memsz());

            char* argv[8] = {
                const_cast<char*>("rd"),
                const_cast<char*>(bs.c_str()),
                const_cast<char*>("-e"),
                const_cast<char*>(es.c_str()),
                const_cast<char*>("|"),
                const_cast<char*>("grep"),
                const_cast<char*>(value),
                nullptr};
            CommandManager::Execute(argv[0], argc, argv);
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, false, false);
}

void CoreSearchCommand::usage() {
    LOGI("Usage: cs <ADDR> [OPTION]\n");
}
