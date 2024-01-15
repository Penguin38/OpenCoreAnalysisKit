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
#include "api/core.h"
#include "ui/ui_thread.h"
#include "work/work_thread.h"
#include "command/command.h"
#include "command/command_manager.h"
#include <unistd.h>
#include <iostream>

class QuitCommand : public Command {
public:
    QuitCommand() : Command("quit", "q") {}
    ~QuitCommand() {}
    void usage() {}
    int main(int /*argc*/, char* const * /*argv[]*/) {
        if (CoreApi::IsReady()) {
            CoreApi::UnLoad();
        }
        _exit(0);
        return 0;
    }
};

class VersionCommand : public Command {
public:
    VersionCommand() : Command("version") {}
    ~VersionCommand() {}
    void usage() {}
    int main(int /*argc*/, char* const * /*argv[]*/) {
        LOGI("1,0\n");
        return 0;
    }
};

int main(int argc, const char* argv[]) {
    CommandManager::Init();
    CommandManager::PushInlineCommand(new VersionCommand());
    CommandManager::PushInlineCommand(new QuitCommand());
    UiThread ui;
    while (1) {
        std::string cmdline;
        ui.GetCommand(&cmdline);
        WorkThread work(cmdline);
        work.Join();
        ui.Wake();
    }
    ui.Join();
    return 0;
}
