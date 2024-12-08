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
#include "command/fake/core/fake_core.h"
#include "command/fake/core/lp64/fake_core.h"
#include "command/fake/core/lp32/fake_core.h"
#include "command/fake/core/arm64/fake_core.h"
#include "command/fake/core/arm/fake_core.h"
#include "command/fake/core/x86_64/fake_core.h"
#include "command/fake/core/x86/fake_core.h"
#include "command/fake/core/riscv64/fake_core.h"
#include "tombstone/tombstone.h"
#include <unistd.h>
#include <getopt.h>

int FakeCore::OptionCore(int argc, char* const argv[]) {

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"tomb",        required_argument, 0, 't'},
        {"rebuild",     no_argument,       0, 'r'},
        {"output",      required_argument, 0, 'o'},
        {"map",         no_argument,       0, 'm'},
    };

    bool tomb = false;
    char* tomb_file = nullptr;
    bool rebuild = false;
    char* output = nullptr;
    bool need_overlay_map = false;

    while ((opt = getopt_long(argc, argv, "t:ro:m",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                tomb = true;
                rebuild = false;
                tomb_file = optarg;
                break;
            case 'r':
                tomb = false;
                rebuild = true;
                break;
            case 'o':
                output = optarg;
                break;
            case 'm':
                need_overlay_map = true;
                break;
        }
    }

    std::unique_ptr<FakeCore> impl;
    std::string filename;
    if (rebuild && CoreApi::IsReady()) {
        if (need_overlay_map) {
            auto callback = [&](LinkMap* map) -> bool {
                if (map->l_name()) {
                    CoreApi::Write(map->l_name(),
                                   (void *)map->name(),
                                   strlen(map->name()) + 1);
                }
                return false;
            };
            CoreApi::ForeachLinkMap(callback);
        }

        filename = CoreApi::GetName();
        impl = FakeCore::Make(CoreApi::Bits());
    } else if (tomb && tomb_file) {
        android::Tombstone tombstone(tomb_file);
        filename = tomb_file;
        impl = FakeCore::Make(tombstone);
        // do nothing
    }

    if (!output || filename == output)
        filename.append(FakeCore::FILE_EXTENSIONS);
    else
        filename = output;

    return impl ? impl->execute(filename.c_str()) : 0;
}

std::unique_ptr<FakeCore> FakeCore::Make(int bits) {
    std::unique_ptr<FakeCore> impl;
    if (bits == 64)
        impl = std::make_unique<lp64::FakeCore>();
    else
        impl = std::make_unique<lp32::FakeCore>();
    return std::move(impl);
}

std::unique_ptr<FakeCore> FakeCore::Make(android::Tombstone& tombstone) {
    std::unique_ptr<FakeCore> impl;
    std::string type = tombstone.ABI();
    if (type == "arm64" || type == "ARM64") {
        impl = std::make_unique<arm64::FakeCore>();
    } else if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::FakeCore>();
    } else if (type == "x86_64" || type == "X86_64") {
        impl = std::make_unique<x86_64::FakeCore>();
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::FakeCore>();
    } else if (type == "riscv64" || type == "RISCV64") {
        impl = std::make_unique<riscv64::FakeCore>();
    }
    return std::move(impl);
}

void FakeCore::Usage() {
    LOGI("Usage: fake core <OPTION...>\n");
    LOGI("Option:\n");
    LOGI("    -t, --tomb <TOMBSTONE>    build tombstone fakecore\n");
    LOGI("    -r, --rebuild             rebuild current environment core\n");
    LOGI("    -m, --map                 overlay linkmap's name on rebuild\n");
    LOGI("    -o, --output <COREFILE>   set current fakecore path\n");
    ENTER();
    LOGI("core-parser> fake core -r\n");
    LOGI("FakeCore: saved [core.opencore.tester_6118_Thread-2_6146_1720691326.fakecore]\n");
}
