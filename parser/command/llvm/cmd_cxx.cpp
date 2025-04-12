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
#include "command/llvm/cmd_cxx.h"
#include "command/core/cmd_read.h"
#include "base/utils.h"
#include "api/core.h"
#include "cxx/string.h"
#include "cxx/vector.h"
#include "cxx/map.h"
#include "cxx/list.h"
#include "cxx/unordered_map.h"
#include "cxx/deque.h"
#include <unistd.h>
#include <getopt.h>

typedef int (*CxxCall)(int argc, char* const argv[]);
struct CxxOption {
    const char* cmd;
    CxxCall call;
};

static CxxOption cxx_option[] = {
    {"string", CxxCommand::DumpCxxString},
    {"vector", CxxCommand::DumpCxxVector},
    {"map", CxxCommand::DumpCxxMap},
    {"unordered_map", CxxCommand::DumpCxxUnOrderedMap},
    {"list", CxxCommand::DumpCxxList},
    {"deque", CxxCommand::DumpCxxDeque},
};

int CxxCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return Command::FINISH;

    if (!(argc > 2)) {
        usage();
        return Command::FINISH;
    }

    return Command::ONCHLD;
}

int CxxCommand::main(int argc, char* const argv[]) {
    int count = sizeof(cxx_option)/sizeof(cxx_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], cxx_option[index].cmd)) {
            return cxx_option[index].call(argc - 1, &argv[1]);
        }
    }
    LOGI("unknown command (%s)\n", argv[1]);
    return 0;
}

int CxxCommand::DumpCxxString(int argc, char* const argv[]) {
    uint64_t addr = Utils::atol(argv[1]) & CoreApi::GetVabitsMask();
    cxx::string target = addr;
    LOGI("%s\n", target.c_str());
    return 0;
}

int CxxCommand::DumpCxxVector(int argc, char* const argv[]) {
    int entry_size = CoreApi::GetPointSize();
    int buffer_size = 0x0;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"entry-size",       required_argument, 0,  'e'},
        {"buffer-size",      required_argument, 0,  's'},
        {0,                  0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "e:s:",
                long_options, &option_index)) != -1) {
        switch(opt) {
            case 'e':
                entry_size = std::atoi(optarg);
                break;
            case 's':
                buffer_size = std::atoi(optarg);
                break;
        }
    }

    uint64_t addr = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
    cxx::vector target = addr;
    target.SetEntrySize(entry_size);
    int idx = 0;
    for (const auto& value : target) {
        LOGI("[%d] 0x%" PRIx64 "\n", idx++, value);
        if (buffer_size)
            ReadCommand::ShowBuffer(value, buffer_size);
    }
    return 0;
}

int CxxCommand::DumpCxxMap(int argc, char* const argv[]) {
    int buffer_size = 0x0;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"buffer-size",      required_argument, 0,  's'},
        {0,                  0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "s:",
                long_options, &option_index)) != -1) {
        switch(opt) {
            case 's':
                buffer_size = std::atoi(optarg);
                break;
        }
    }

    uint64_t addr = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
    cxx::map target = addr;
    int idx = 0;
    for (const auto& value : target) {
        LOGI("[%d] 0x%" PRIx64 "\n", idx++, value);
        if (buffer_size)
            ReadCommand::ShowBuffer(value, buffer_size);
    }
    return 0;
}

int CxxCommand::DumpCxxUnOrderedMap(int argc, char* const argv[]) {
    int buffer_size = 0x0;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"buffer-size",      required_argument, 0,  's'},
        {0,                  0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "s:",
                long_options, &option_index)) != -1) {
        switch(opt) {
            case 's':
                buffer_size = std::atoi(optarg);
                break;
        }
    }

    uint64_t addr = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
    cxx::unordered_map target = addr;
    int idx = 0;
    for (const auto& value : target) {
        LOGI("[%d] 0x%" PRIx64 "\n", idx++, value);
        if (buffer_size)
            ReadCommand::ShowBuffer(value, buffer_size);
    }
    return 0;
}

int CxxCommand::DumpCxxList(int argc, char* const argv[]) {
    int buffer_size = 0x0;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"buffer-size",      required_argument, 0,  's'},
        {0,                  0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "s:",
                long_options, &option_index)) != -1) {
        switch(opt) {
            case 's':
                buffer_size = std::atoi(optarg);
                break;
        }
    }

    uint64_t addr = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
    cxx::list target = addr;
    int idx = 0;
    for (const auto& value : target) {
        LOGI("[%d] 0x%" PRIx64 "\n", idx++, value);
        if (buffer_size)
            ReadCommand::ShowBuffer(value, buffer_size);
    }
    return 0;
}

int CxxCommand::DumpCxxDeque(int argc, char* const argv[]) {
    int block_size = CoreApi::GetPointSize();
    int buffer_size = 0x0;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"block-size",       required_argument, 0,  'b'},
        {"buffer-size",      required_argument, 0,  's'},
        {0,                  0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "b:s:",
                long_options, &option_index)) != -1) {
        switch(opt) {
            case 'b':
                block_size = std::atoi(optarg);
                break;
            case 's':
                buffer_size = std::atoi(optarg);
                break;
        }
    }

    uint64_t addr = Utils::atol(argv[optind]) & CoreApi::GetVabitsMask();
    cxx::deque target = addr;
    target.SetBlockSize(block_size);
    int idx = 0;
    for (const auto& value : target) {
        LOGI("[%d] 0x%" PRIx64 "\n", idx++, value.Ptr());
        if (buffer_size)
            ReadCommand::ShowBuffer(value.Ptr(), buffer_size);
    }
    return 0;
}

void CxxCommand::usage() {
    LOGI("Usage: cxx <TYPE> <ADDR> [OPTION]\n");
    LOGI("Type:\n");
    LOGI("    string          vector    map\n");
    LOGI("    unordered_map   list      deque\n");
    LOGI("Option:\n");
    LOGI("    -e, --entry-size    only vector set entry-size\n");
    LOGI("    -b, --block-size    only deque set block-size\n");
    LOGI("    -s, --buffer-size   show target pointer near memory\n");
    ENTER();
    LOGI("core-parser> cxx string 0x79191ce66ed8\n");
    LOGI("/apex/com.android.art/javalib/x86_64/boot-okhttp.art\n");
    ENTER();
    LOGI("core-parser> cxx vector 0x79196ce6d3c0 --entry-size 8\n");
    LOGI("[0] 0x7918cce67d80\n");
    LOGI("[1] 0x7918cce67d88\n");
    LOGI("[2] 0x7918cce67d90\n");
    LOGI("[3] 0x7918cce67d98\n");
    LOGI("[4] 0x7918cce67da0\n");
    LOGI("[5] 0x7918cce67da8\n");
    LOGI("[6] 0x7918cce67db0\n");
    LOGI("[7] 0x7918cce67db8\n");
    LOGI("[8] 0x7918cce67dc0\n");
    LOGI("[9] 0x7918cce67dc8\n");
    LOGI("[10] 0x7918cce67dd0\n");
    LOGI("[11] 0x7918cce67dd8\n");
    LOGI("[12] 0x7918cce67de0\n");
    LOGI("[13] 0x7918cce67de8\n");
    LOGI("[14] 0x7918cce67df0\n");
}
