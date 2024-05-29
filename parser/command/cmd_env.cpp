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
#include "android.h"
#include "command/cmd_env.h"
#include "command/env.h"
#include "api/core.h"
#include "base/utils.h"
#include <unistd.h>
#include <getopt.h>

typedef int (*EnvCall)(int argc, char* const argv[]);
struct EnvOption {
    const char* cmd;
    EnvCall call;
};

EnvOption env_option[] = {
    { "config", EnvCommand::onConfigChanged },
    { "logger", EnvCommand::onLoggerChanged },
    { "art", EnvCommand::showArtEnv },
    { "load", EnvCommand::showLoadEnv },
};

int EnvCommand::main(int argc, char* const argv[]) {
    if (!(argc > 1))
        return dumpEnv();

    int count = sizeof(env_option)/sizeof(env_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], env_option[index].cmd)) {
            return env_option[index].call(argc - 1, &argv[1]);
        }
    }

    LOGI("unknown command (%s)\n", argv[1]);
    return 0;
}

int EnvCommand::onConfigChanged(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int opt;
    int option_index = 0;
    int current_sdk = Android::UPSIDE_DOWN_CAKE;
    int current_pid = Env::CurrentPid();
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {"sdk",     required_argument, 0,  0 },
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "p:0:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                current_pid = atoi(optarg);
                if (Env::SetCurrentPid(current_pid))
                    Env::Dump();
                break;
            case 0:
                current_sdk = atoi(optarg);
                Android::OnSdkChanged(current_sdk);
                break;
        }
    }

    return 0;
}

int EnvCommand::onLoggerChanged(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"debug",   no_argument,       0, Logger::LEVEL_DEBUG},
        {"info",    no_argument,       0, Logger::LEVEL_INFO},
        {"warn",    no_argument,       0, Logger::LEVEL_WARN},
        {"error",   no_argument,       0, Logger::LEVEL_ERROR},
        {"fatal",   no_argument,       0, Logger::LEVEL_FATAL},
        {0,         0,                 0,  0 }
    };

    if (argc < 2) {
        LOGI("Current logger level %s\n", long_options[Logger::GetLevel()].name);
        return 0;
    }

    while ((opt = getopt_long(argc, argv, "01234",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                LOGI("Switch logger level %s\n", long_options[option_index].name);
                Logger::SetLevel(opt);
                break;
            default:
                LOGI("Unkown logger level.\n");
                break;
        }
    }
    return 0;
}

int EnvCommand::showArtEnv(int argc, char* const argv[]) {
    if (!Android::IsReady())
        return 0;

    LOGI("  * LIB: %s\n", Android::GetRealLibart().c_str());
    art::Runtime& runtime = Android::GetRuntime();
    LOGI("  * art::Runtime: 0x%lx\n", runtime.Ptr());
    if (!runtime.Ptr())
        return 0;

    LOGI("  * art::gc::Heap: 0x%lx\n", runtime.GetHeap().Ptr());
    LOGI("  * art::ThreadList: 0x%lx\n", runtime.GetThreadList().Ptr());
    LOGI("  * art::ClassLinker: 0x%lx\n", runtime.GetClassLinker().Ptr());
    LOGI("  * art::JavaVMExt: 0x%lx\n", runtime.GetJavaVM().Ptr());
    return 0;
}

int EnvCommand::showLoadEnv(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int index = 0;
    auto callback = [&index](LoadBlock *block) -> bool {
        std::string name;
        if (block->isMmapBlock()) {
            name = block->name();
        } else {
            name.append("[]");
        }
        std::string valid;
        if (block->isValid()) {
            valid.append("[*]");
            if (block->isOverlayBlock()) {
                valid.append("(OVERLAY)");
            } else if (block->isMmapBlock()) {
                valid.append("(MMAP");
                if (block->GetMmapOffset()) {
                    valid.append(" ");
                    valid.append(Utils::ToHex(block->GetMmapOffset()));
                }
                valid.append(")");
            }
        } else {
            valid.append("[EMPTY]");
        }
        LOGI("  %-5d [%lx, %lx)  %s  %010lx  %s %s\n", index++,
                block->vaddr(), block->vaddr() + block->size(), block->convertFlags().c_str(),
                block->realSize(), name.c_str(), valid.c_str());
        return false;
    };
    LOGI("INDEX   REGION               FLAGS FILESZ      PATH\n");
    CoreApi::ForeachLoadBlock(callback, false);
    return 0;
}

int EnvCommand::dumpEnv() {
    if (CoreApi::IsReady()) {
        CoreApi::Dump();
        Env::Dump();
    }

    if (Android::IsReady()) {
        Android::Dump();
    }

    return 0;
}

void EnvCommand::usage() {
    LOGI("Usage: env <COMMAND> [option] ...\n");
    LOGI("Command:\n");
    LOGI("    config  logger  art  load\n");
    LOGI("\n");
    LOGI("Usage: env config <option> ..\n");
    LOGI("Option:\n");
    LOGI("   --sdk: <VERSION>\n");
    LOGI("   --pid|-p <PID>\n");
    LOGI("\n");
    LOGI("Usage: env logger <option>\n");
    LOGI("Option:\n");
    LOGI("   --[debug|info|warn|error|fatal]\n");
    LOGI("\n");
    LOGI("Usage: env art: show art env args\n");
    LOGI("Usage env load: show code load segments\n");
}
