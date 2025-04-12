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
#include "base/utils.h"
#include "api/core.h"
#include "common/bit.h"
#include "common/elf.h"
#include "command/env.h"
#include "command/remote/cmd_remote.h"
#include "command/remote/opencore/opencore.h"
#include "command/remote/hook/hook.h"
#include "command/remote/setprop/property.h"
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <memory>

typedef int (*RemoteCall)(int argc, char* const argv[]);
struct RemoteOption {
    const char* cmd;
    RemoteCall call;
};

static RemoteOption remote_option[] = {
    { "core",   Opencore::Dump },
    { "hook",   Hook::Main },
    { "rd",     RemoteCommand::OptionRead },
    { "wd",     RemoteCommand::OptionWrite },
    { "pause",  RemoteCommand::OptionPause },
#if defined(__ANDROID__)
    { "setprop",AndroidProperty::Main },
#endif
    { "maps",   RemoteCommand::OptionMaps },
};

int RemoteCommand::prepare(int argc, char* const argv[]) {
    if (!(argc > 1)) {
        usage();
        return Command::FINISH;
    }

    return Command::ONCHLD;
}

int RemoteCommand::main(int argc, char* const argv[]) {
    struct sigaction stact;
    memset(&stact, 0, sizeof(stact));
    stact.sa_handler = Opencore::TermStopHandle;
    sigaction(SIGINT, &stact, NULL);
    sigaction(SIGTERM, &stact, NULL);

    int count = sizeof(remote_option)/sizeof(remote_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], remote_option[index].cmd)) {
            return remote_option[index].call(argc - 1, &argv[1]);
        }
    }
    LOGI("unknown command (%s)\n", argv[1]);
    return 0;
}

int RemoteCommand::OptionRead(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {0,         0,                 0,  0 },
    };

    int pid = 0;
    if (CoreApi::IsRemote())
        pid = Env::CurrentRemotePid();
    uint64_t end = 0;
    while ((opt = getopt_long(argc, argv, "p:e:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                pid = std::atoi(optarg);
                break;
            case 'e':
                end = Utils::atol(optarg);
                break;
        }
    }

    if (optind >= argc) {
        return 0;
    }

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(pid);

    uint64_t value[ELF_PAGE_SIZE / 8];
    memset(value, 0x0, ELF_PAGE_SIZE);

    uint64_t begin = Utils::atol(argv[optind]);
    if (!RemoteCommand::Read(pid, begin, sizeof(value), (uint8_t *)&value))
        return 0;

    int count = RoundUp((end - begin) / 8, 2);
    if (begin >= end || !count) {
        std::string ascii = Utils::ConvertAscii(*value, 8);
        LOGI(ANSI_COLOR_CYAN "%" PRIx64 "" ANSI_COLOR_RESET ": %016" PRIx64 "  %s\n", begin, (*value), ascii.c_str());
    } else {
        for (int i = 0; i < count && i < 512; i += 2) {
            LOGI(ANSI_COLOR_CYAN "%" PRIx64 "" ANSI_COLOR_RESET ": %016" PRIx64 "  %016" PRIx64 "  %s%s\n", (begin + i * 8), value[i], value[i + 1],
                    Utils::ConvertAscii(value[i], 8).c_str(), Utils::ConvertAscii(value[i + 1], 8).c_str());
        }
    }
    return 0;
}

int RemoteCommand::OptionWrite(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {"string",  required_argument, 0, 's'},
        {"value",   required_argument, 0, 'v'},
        {0,         0,                 0,  0 },
    };

    int pid = 0;
    if (CoreApi::IsRemote())
        pid = Env::CurrentRemotePid();
    char* buf = nullptr;
    uint64_t value = 0x0;
    while ((opt = getopt_long(argc, argv, "s:v:p:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                pid = std::atoi(optarg);
                break;
            case 's':
                buf = optarg;
                break;
            case 'v':
                value = Utils::atol(optarg);
                break;
        }
    }

    if (optind >= argc) {
        return 0;
    }

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(pid);

    if (buf) {
        RemoteCommand::Write(pid, Utils::atol(argv[optind]), (void *)buf, strlen(buf) + 1);
    } else {
        RemoteCommand::Write(pid, Utils::atol(argv[optind]), value);
    }

    return 0;
}

void RemoteCommand::Write(int pid, uint64_t vaddr, void *buf, uint64_t size) {
#ifdef __ARM64__
    vaddr &= ((1ULL << 56) - 1);
#endif
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        LOGE("open %s fail.\n", filename);
        return;
    }

    if (pwrite64(fd, buf, size, vaddr) < 0)
        LOGE("write %" PRIx64 " fail.\n", vaddr);

    close(fd);
}

bool RemoteCommand::Read(int pid, uint64_t vaddr, uint64_t size, uint8_t* buf) {
#ifdef __ARM64__
    vaddr &= ((1ULL << 56) - 1);
#endif
    char filename[32];
    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        LOGE("open %s fail.\n", filename);
        return false;
    }

    if (pread64(fd, buf, size, vaddr) < 0) {
        LOGE("read %" PRIx64 " fail.\n", vaddr);
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

int RemoteCommand::OptionPause(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"all", no_argument, 0, 'a'},
        {0,     0,           0,  0 },
    };

    bool all = false;
    while ((opt = getopt_long(argc, argv, "a",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                all = true;
                break;
        }
    }

    if (optind >= argc) {
        return 0;
    }

    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    for (int i = optind; i < argc; ++i) {
        int tid = std::atoi(argv[i]);
        if (all)
            opencore->StopTheWorld(tid);
        else
            opencore->StopTheThread(tid);
    }

    LOGI("please enter any key cancel.\n");
    getchar();
    return 0;
}

int RemoteCommand::OptionMaps(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {0,         0,                 0,  0 },
    };

    int pid = 0;
    if (CoreApi::IsRemote())
        pid = Env::CurrentRemotePid();
    while ((opt = getopt_long(argc, argv, "p:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                pid = std::atoi(optarg);
                break;
        }
    }

    std::vector<Opencore::VirtualMemoryArea> maps;
    Opencore::ParseMaps(pid, maps);

    for (const auto& vma : maps) {
        LOGI("%016" PRIx64 "-%016" PRIx64 "  %c%c%c%c  %08x  %02x:%02x  %" PRIu64 "  %s\n",
            vma.begin, vma.end, vma.flags[0], vma.flags[1], vma.flags[2], vma.flags[3],
            vma.offset, vma.major, vma.minor, vma.inode, vma.file.c_str());
    }
    return 0;
}

void RemoteCommand::usage() {
    LOGI("Usage: remote <COMMAND> [OPTION...]\n");
    LOGI("Command:\n");
    LOGI("    core      hook      rd      wd\n");
    LOGI("    pause     setprop   maps\n");
    ENTER();
    Opencore::Usage();
    ENTER();
    Hook::Usage();
    ENTER();
    LOGI("remote wd [-p <PID>] <ADDRESS> [-s|-v] <VALUE>\n");
    LOGI("core-parser> remote wd -p 1 7fb989794000 -s PenguinLetsGo\n");
    ENTER();
    LOGI("remote rd [-p <PID>] <BEGIN_ADDR> -e <END_ADDR>\n");
    LOGI("core-parser> remote rd -p 1 7fb989794000 -e 7fb989794030\n");
    LOGI("7fb989794000: 4c6e6975676e6550  0000006f47737465  PenguinLetsGo...\n");
    LOGI("7fb989794010: 00000001003e0003  0000000000068ab0  ..>.............\n");
    LOGI("7fb989794020: 0000000000000040  0000000000198c20  @...............\n");
    ENTER();
    LOGI("remote pause <PID ...> [-a]\n");
#if defined(__ANDROID__)
    ENTER();
    AndroidProperty::Usage();
#endif
}
