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
#include <getopt.h>
#include <signal.h>
#include <iostream>

void show_copyright() {
    LOGI("Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.\n\n");

    LOGI("Licensed under the Apache License, Version 2.0 (the \"License\");\n");
    LOGI("you may not use this file ercept in compliance with the License.\n");
    LOGI("You may obtain a copy of the License at\n\n");

    LOGI("     http://www.apache.org/licenses/LICENSE-2.0\n\n");

    LOGI("Unless required by applicable law or agreed to in writing, software\n");
    LOGI("distributed under the License is distributed on an \"AS IS\" BASIS,\n");
    LOGI("WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.\n");
    LOGI("See the License for the specific language governing permissions and\n");
    LOGI("limitations under the License.\n\n");

    LOGI("For bug reporting instructions, please see:\n");
    LOGI("     https://github.com/Penguin38/OpenCoreAnalysisKit\n\n");
}

void show_compat_android_version() {
    LOGI("-------------------------------------------------------------------\n");
    LOGI("| SDK           |  arm64  |   arm   |  x86_64 |   x86   | riscv64 |\n");
    LOGI("|---------------|---------|---------|---------|---------|---------|\n");
    LOGI("| AOSP-8.0 (26) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-8.1 (27) |    -    |    -    |    -    |    -    |    -    |\n");
    LOGI("| AOSP-9.0 (28) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-10.0(29) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-11.0(30) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-12.0(31) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-12.1(32) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-13.0(33) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-14.0(34) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-15.0(35) |    √    |    -    |    √    |    -    |    ?    |\n");
    LOGI("-------------------------------------------------------------------\n\n");
}

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
        LOGI("(%s) core-parser 1.0\n", __TARGET_PARSER__);
        return 0;
    }
};

int command_preload(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"core",  required_argument,       0, 'c'},
        {"sdk",   required_argument,       0,  1 },
        {0,       0,                       0,  0 },
    };

    while ((opt = getopt_long(argc, argv, "c:1:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c': {
                std::string cmdline;
                cmdline.append("core ");
                cmdline.append(optarg);
                WorkThread work(cmdline);
                work.Join();
            } break;
            case 1: {
                std::string cmdline;
                cmdline.append("env config --sdk ");
                cmdline.append(optarg);
                WorkThread work(cmdline);
                work.Join();
            } break;
        }
    }

    // reset
    optind = 0;
    return 0;
}

int main(int argc, char* const argv[]) {
    struct sigaction stact;
    memset(&stact, 0, sizeof(stact));
    stact.sa_handler = WorkThread::Stop;
    sigaction(SIGINT, &stact, NULL);
    sigaction(SIGTERM, &stact, NULL);

    CommandManager::Init();
    CommandManager::PushInlineCommand(new VersionCommand());
    CommandManager::PushInlineCommand(new QuitCommand());

    show_copyright();
#if defined(__AOSP_PARSER__)
    show_compat_android_version();
#endif
    command_preload(argc, argv);

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
