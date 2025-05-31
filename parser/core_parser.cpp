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
#include "api/core.h"
#include "ui/ui_thread.h"
#include "work/work_thread.h"
#include "android.h"
#include "common/elf.h"
#include "command/env.h"
#include "command/core/cmd_core.h"
#include "command/command.h"
#include "command/command_manager.h"
#include "command/remote/opencore/opencore.h"
#include "command/fake/core/fake_core.h"
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <iostream>

void show_copyright() {
    LOGI(ANSI_COLOR_LIGHTRED "Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.\n\n" ANSI_COLOR_RESET);

    LOGI("Licensed under the Apache License, Version 2.0 (the \"License\");\n");
    LOGI("you may not use this file except in compliance with the License.\n");
    LOGI("You may obtain a copy of the License at\n\n");

    LOGI(ANSI_COLOR_LIGHTGREEN "     http://www.apache.org/licenses/LICENSE-2.0\n\n" ANSI_COLOR_RESET);

    LOGI("Unless required by applicable law or agreed to in writing, software\n");
    LOGI("distributed under the License is distributed on an \"AS IS\" BASIS,\n");
    LOGI("WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n");
    LOGI("See the License for the specific language governing permissions and\n");
    LOGI("limitations under the License.\n\n");

    LOGI("For bug reporting instructions, please see:\n");
    LOGI(ANSI_COLOR_LIGHTGREEN "     https://github.com/Penguin38/OpenCoreAnalysisKit\n\n" ANSI_COLOR_RESET);
}

void show_compat_android_version() {
    LOGI("+-----------------------------------------------------------------+\n");
    LOGI("| SDK           |  arm64  |   arm   |  x86_64 |   x86   | riscv64 |\n");
    LOGI("|---------------|---------|---------|---------|---------|---------|\n");
    LOGI("| AOSP-5.0 (21) |    x    |    x    |    x    |    x    |    ?    |\n");
    LOGI("| AOSP-5.1 (22) |    x    |    x    |    x    |    x    |    ?    |\n");
    LOGI("| AOSP-6.0 (23) |    x    |    x    |    x    |    x    |    ?    |\n");
    LOGI("| AOSP-7.0 (24) |    √    |    √    |    √    |    √    |    ?    |\n");
    LOGI("| AOSP-7.1 (25) |    -    |    -    |    -    |    -    |    -    |\n");
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
    LOGI("| AOSP-16.0(36) |    √    |    -    |    √    |    -    |    ?    |\n");
    LOGI("+-----------------------------------------------------------------+\n\n");
}

void show_parser_usage() {
    LOGI("Usage: core-parser [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -c, --core <COREFILE>    load core-parser from corefile\n");
#if !defined(__MACOS__)
    LOGI("    -p, --pid <PID>          load core-parser from target process\n");
    LOGI("    -f, --filter <Filter>    set coredump ignore filter\n");
    LOGI("Filter: (0x19 default)\n");
    LOGI("     0x001: filter-special-vma (default)\n");
    LOGI("     0x002: filter-file-vma\n");
    LOGI("     0x004: filter-shared-vma\n");
    LOGI("     0x008: filter-sanitizer-shadow-vma (default)\n");
    LOGI("     0x010: filter-non-read-vma (default)\n");
    LOGI("     0x020: filter-signal-context (unused)\n");
    LOGI("     0x040: filter-minidump\n");
    LOGI("     0x080: filter-javaheap-vma\n");
    LOGI("     0x100: filter-jit-cache-vma\n");
#endif
    LOGI("    -m, --machine <ARCH>     arch support arm64, arm, x86_64, x86, riscv64\n");
    LOGI("        --sdk <SDK>          sdk support 26 ~ 36\n");
    LOGI("        --no-filter-any      load core-parser no filter any vma\n");
    LOGI("    -t, --tomb <TOMBSTONE>   load core-parser form tombstone file\n");
    LOGI("        --sysroot <DIR:DIR>  set sysroot path\n");
    LOGI("        --va_bits <BITS>     set virtual valid addr bits\n");
    LOGI("        --page_size <SIZE>   set target core page size\n");
    LOGI("        --no-load            no auto load corefile\n");
    LOGI("        --no-fake-phdr [EXE] rebuild fakecore phdr\n");
    LOGI("    -d, --debug <LEVEL>      set logger debug level\n");
    LOGI("Exp:\n");
    LOGI("    core-parser -c /tmp/tmp.core\n");
#if !defined(__MACOS__)
    LOGI("    core-parser -p 1 -m arm64\n");
#endif
    LOGI("    core-parser -t tombstone_00 --sysroot symbols\n");
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

int command_preload(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"core",      required_argument,   0, 'c'},
        {"filter",    required_argument,   0, 'f'},
        {"sdk",       required_argument,   0,  1 },
#if !defined(__MACOS__)
        {"pid",       required_argument,   0, 'p'},
#endif
        {"tomb",      required_argument,   0, 't'},
        {"sysroot",   required_argument,   0,  3 },
        {"va_bits",   required_argument,   0,  4 },
        {"page_size", required_argument,   0,  5 },
        {"machine",   required_argument,   0, 'm'},
        {"no-filter-any", no_argument,     0,  2 },
        {"no-load",   no_argument,         0,  6 },
        {"no-fake-phdr",no_argument,       0,  7 },
        {"debug",     required_argument,   0, 'd'},
        {"help",      no_argument,         0, 'h'},
        {0,           0,                   0,  0 },
    };

    char* corefile = nullptr;
    char* machine = const_cast<char *>(NONE_MACHINE);
    char* tombstone = nullptr;
    char* sysroot = nullptr;
    uint64_t page_size = 0;
    uint64_t va_bits = 0;
    int current_sdk = 0;
    int pid = 0;
    bool remote = false;
    bool need_load = true;
    bool no_fake_phdr = false;
    int lv = 0;
    int filter = Opencore::FILTER_SPECIAL_VMA
               | Opencore::FILTER_SANITIZER_SHADOW_VMA
               | Opencore::FILTER_NON_READ_VMA;

    while ((opt = getopt_long(argc, argv, "c:f:1:p:m:t:d:h",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                corefile = optarg;
                need_load = true;
                break;
            case 'f':
                filter = Utils::atol(optarg);
                break;
            case 1:
                current_sdk = std::atoi(optarg);
                break;
            case 'm':
                machine = optarg;
                break;
            case 'p':
                remote = true;
                pid = std::atoi(optarg);
                break;
            case 2:
                CoreApi::QUICK_LOAD_ENABLED = false;
                break;
            case 't':
                tombstone = optarg;
                break;
            case 3:
                sysroot = optarg;
                break;
            case 4:
                va_bits = std::atoi(optarg);
                CoreApi::VA_BITS = va_bits;
                break;
            case 5:
                page_size = Utils::atol(optarg);
                break;
            case 6:
                if (!corefile) need_load = false;
                break;
            case 7:
                no_fake_phdr = true;
                break;
            case 'd':
                lv = std::atoi(optarg);
                if (lv >= Logger::LEVEL_NONE && lv <= Logger::LEVEL_DEBUG_2) {
                    LOGI("Switch logger debug level: %d\n", lv);
                    Logger::SetDebugLevel(lv);
                }
                break;
            case 'h':
                show_parser_usage();
                return -1;
        }
    }

    std::string output;
    if (pid) {
#if !defined(__MACOS__)
        std::string cmdline;
        cmdline.append("remote core -p ");
        cmdline.append(std::to_string(pid));
        cmdline.append(" -f ");
        cmdline.append(Utils::ToHex(filter));
        if (strcmp(machine, NONE_MACHINE)) {
            cmdline.append(" -m ");
            cmdline.append(machine);
        }
        output = Env::CurrentDir();
        std::string file = std::to_string(pid) + ".core";
        output += "/" + file;
        cmdline.append(" -o ");
        cmdline.append(file);;
        WorkThread work(cmdline);
        work.Join();
        corefile = output.data();
#endif
    } else if (tombstone) {
        std::string cmdline;
        cmdline.append("fake core -t ");
        cmdline.append(tombstone);
        if (sysroot) {
            cmdline.append(" --sysroot ");
            cmdline.append(sysroot);
        }
        if (page_size) {
            cmdline.append(" --page_size ");
            cmdline.append(Utils::ToHex(page_size));
        }
        if (va_bits) {
            cmdline.append(" --va_bits ");
            cmdline.append(std::to_string(va_bits));
        }
        if (no_fake_phdr) {
            cmdline.append(" --no-fake-phdr ");
            if (optind < argc) cmdline.append(argv[optind]);
        }
        output = tombstone;
        output.append(FakeCore::FILE_EXTENSIONS);
        WorkThread work(cmdline);
        work.Join();
        corefile = output.data();
    }

    if (corefile && need_load) {
        if (CoreCommand::Load(corefile, remote)) {
#if defined(__AOSP_PARSER__)
            if (current_sdk) Android::OnSdkChanged(current_sdk);
#endif
            if (sysroot) CoreApi::SysRoot(sysroot);
        }
    }

    // reset
    optind = 0;
    return need_load? 0 : -1;
}

int main(int argc, char* const argv[]) {
    struct sigaction stact;
    memset(&stact, 0, sizeof(stact));
    stact.sa_handler = WorkThread::Stop;
    sigaction(SIGINT, &stact, NULL);
    sigaction(SIGTERM, &stact, NULL);

    CommandManager::Init();
    CommandManager::PushInlineCommand(new QuitCommand());

    show_copyright();
#if defined(__AOSP_PARSER__)
    show_compat_android_version();
#endif
    if (command_preload(argc, argv) < 0) {
        return 0;
    }

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
