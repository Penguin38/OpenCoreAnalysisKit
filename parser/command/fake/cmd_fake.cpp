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
#include "command/fake/cmd_fake.h"
#include "command/fake/core/fake_core.h"
#include "command/fake/map/fake_map.h"
#include "command/fake/exec/fake_executable.h"
#include "command/fake/load/fake_load_block.h"
#include "command/fake/stack/fake_java_stack.h"
#include <unistd.h>
#include <getopt.h>

typedef int (*FakeCall)(int argc, char* const argv[]);
struct FakeOption {
    const char* cmd;
    FakeCall call;
    bool onbg;
};

static FakeOption fake_option[] = {
    { "core", FakeCore::OptionCore, true },
    { "map", FakeLinkMap::OptionMap, false },
    { "exec", FakeExecutable::OptionExec, false },
    { "stack", FakeJavaStack::OptionJavaStack, false },
    { "load", FakeLoadBlock::OptionLoad, false },
};

int FakeCommand::prepare(int argc, char* const argv[]) {
    if (!(argc > 1)) {
        usage();
        return Command::FINISH;
    }

    int count = sizeof(fake_option)/sizeof(fake_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], fake_option[index].cmd)) {
            return fake_option[index].onbg ? Command::ONCHLD : Command::CONTINUE;
        }
    }

    return Command::FINISH;
}

int FakeCommand::main(int argc, char* const argv[]) {
    int count = sizeof(fake_option)/sizeof(fake_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], fake_option[index].cmd)) {
            return fake_option[index].call(argc - 1, &argv[1]);
        }
    }
    LOGI("unknown command (%s)\n", argv[1]);
    return 0;
}

void FakeCommand::usage() {
    LOGI("Usage: fake <COMMAND> [OPTION...]\n");
    LOGI("Command:\n");
    LOGI("    core    map    exec    load    stack\n");
    ENTER();
    FakeCore::Usage();
    ENTER();
    FakeLinkMap::Usage();
    ENTER();
    FakeExecutable::Usage();
    ENTER();
    FakeLoadBlock::Usage();
    ENTER();
    FakeJavaStack::Usage();
}
