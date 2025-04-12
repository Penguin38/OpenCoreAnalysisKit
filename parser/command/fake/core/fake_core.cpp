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
#include "base/utils.h"
#include "command/fake/core/fake_core.h"
#include "command/fake/core/lp64/fake_core.h"
#include "command/fake/core/lp32/fake_core.h"
#include "command/fake/core/arm64/fake_core.h"
#include "command/fake/core/arm/fake_core.h"
#include "command/fake/core/x86_64/fake_core.h"
#include "command/fake/core/x86/fake_core.h"
#include "command/fake/core/riscv64/fake_core.h"
#include "command/fake/core/tombstone.h"
#include <unistd.h>
#include <getopt.h>

int FakeCore::OptionCore(int argc, char* const argv[]) {

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"tomb",        required_argument, 0, 't'},
        {"rebuild",     no_argument,       0, 'r'},
        {"sysroot",     required_argument, 0,  1 },
        {"va_bits",     required_argument, 0,  2 },
        {"page_size",   required_argument, 0,  3 },
        {"no-fake-phdr",no_argument,       0,  4 },
        {"output",      required_argument, 0, 'o'},
        {"map",         no_argument,       0, 'm'},
        {0,             0,                 0,  0 },
    };

    bool tomb = false;
    char* tomb_file = nullptr;
    bool rebuild = false;
    char* output = nullptr;
    bool need_overlay_map = false;
    char* sysroot_dir = nullptr;
    uint64_t page_size = ELF_PAGE_SIZE;
    uint64_t va_bits = 0;
    uint32_t fake_mask = 0;

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
            case 1:
                sysroot_dir = optarg;
                break;
            case 2:
                va_bits = std::atoi(optarg);
                break;
            case 3:
                page_size = Utils::atol(optarg);
                break;
            case 4:
                fake_mask |= NO_FAKE_PHDR;
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
        std::unique_ptr<FakeCore::Stream> tombstone =
                std::make_unique<android::Tombstone>(tomb_file);
        filename = tomb_file;
        impl = FakeCore::Make(tombstone);
        if (impl) {
            impl->InitSysRoot(sysroot_dir);
            impl->InitVaBits(va_bits);
            impl->InitPageSize(page_size);
            impl->InitMask(fake_mask);
            if (optind < argc) {
                impl->InitExecutable(argv[optind]);
                impl->GetInputStream()->SetExecutable(argv[optind]);
            }
        }
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

std::unique_ptr<FakeCore> FakeCore::Make(std::unique_ptr<FakeCore::Stream>& stream) {
    std::unique_ptr<FakeCore> impl;
    std::string type = stream->ABI();
#if defined(__LP64__)
    if (type == "arm64" || type == "ARM64") {
        impl = std::make_unique<arm64::FakeCore>(stream);
    } else if (type == "x86_64" || type == "X86_64") {
        impl = std::make_unique<x86_64::FakeCore>(stream);
    } else if (type == "riscv64" || type == "RISCV64") {
        impl = std::make_unique<riscv64::FakeCore>(stream);
    } else if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::FakeCore>(stream);
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::FakeCore>(stream);
    }
#else
    if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::FakeCore>(stream);
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::FakeCore>(stream);
    }
#endif
    return std::move(impl);
}

uint64_t FakeCore::FindModuleLoad(std::vector<Opencore::VirtualMemoryArea>& maps, const char* name) {
    if (!name) return 0;

    uint64_t module_load = 0x0;
    std::string lib = name;
    std::size_t pos = lib.find(":");
    std::string libname;
    std::string buildid;

    if (pos != std::string::npos) {
        libname = lib.substr(0, pos);
        buildid = lib.substr(pos + 1, 32);
    } else {
        libname = name;
    }

    std::vector<Opencore::VirtualMemoryArea> tmps;
    for (const auto& vma : maps) {
        if (vma.file != libname
                || (buildid.length() && vma.buildid != buildid))
            continue;

        if (vma.flags[2] == 'x' || vma.flags[2] == 'X') {
            if (tmps.size() == 0) {
                module_load = vma.begin;
                break;
            }
            for (const auto& phdr : tmps) {
                uint64_t cloc_vaddr = vma.begin - vma.offset + phdr.offset;
                if (phdr.begin > cloc_vaddr)
                    continue;

                if (phdr.begin <= cloc_vaddr)
                    module_load = phdr.begin;

                if (phdr.begin == cloc_vaddr)
                    break;
            }
        } else
            tmps.push_back(vma);

        if (module_load)
            break;
    }

    LOGI("0x%" PRIx64 " %s\n", module_load, name);
    return module_load;
}

void FakeCore::Usage() {
    LOGI("Usage: fake core <OPTION...>\n");
    LOGI("Option:\n");
    LOGI("    -t, --tomb <TOMBSTONE>    build tombstone fakecore\n");
    LOGI("        --sysroot <DIR:DIR>   set sysroot path\n");
    LOGI("        --va_bits <BITS>      set virtual invalid addr bits\n");
    LOGI("        --page_size <SIZE>    set target core page size\n");
    LOGI("        --no-fake-phdr [EXE]  rebuild fakecore phdr\n");
    LOGI("    -r, --rebuild             rebuild current environment core\n");
    LOGI("    -m, --map                 overlay linkmap's name on rebuild\n");
    LOGI("    -o, --output <COREFILE>   set current fakecore path\n");
    ENTER();
    LOGI("core-parser> fake core -r\n");
    LOGI("FakeCore: saved [core.opencore.tester_6118_Thread-2_6146_1720691326.fakecore]\n");
    ENTER();
    LOGI("core-parser> fake core -t fakecore/tombstone_09 --sysroot fakecore/symbols\n");
    LOGI("Tid: 25911\n");
    LOGI("tagged_addr_ctrl 1\n");
    LOGI("pac_enabled_keys f\n");
    LOGI("x0  0x0000000000000000  x1  0x0000000000000001  x2  0x0000007d438049e0  x3  0x0000007d3d9525e8  \n");
    LOGI("x4  0x0000000000000001  x5  0x0000000000000000  x6  0x0000000000000048  x7  0x0000007d4551dae0  \n");
    LOGI("x8  0x0000000000000000  x9  0x4800abef36f3d649  x10 0x0000000000000000  x11 0x0000007d3d953878  \n");
    LOGI("x12 0x000000000000001c  x13 0x0000000000000000  x14 0xffffffffe0000000  x15 0x0000000010000000  \n");
    LOGI("x16 0x0000000000000001  x17 0x0000007e0c764f18  x18 0x0000007cec46a000  x19 0xb400007d43666d68  \n");
    LOGI("x20 0xb400007b619cde48  x21 0xb400007d43666d78  x22 0x0000007d619ff238  x23 0xb400007b6191bba0  \n");
    LOGI("x24 0x0000007d4551f000  x25 0xb400007b61926690  x26 0x0000000000000003  x27 0x0000007d619ff220  \n");
    LOGI("x28 0xb400007b61a12690  fp  0x0000007d4551ddb0  \n");
    LOGI("lr  0x0000007d619aa960  sp  0x0000007d4551dd70  pc  0x0000007d619aa89c  pst 0x0000000080001000  \n");
    LOGI("/apex/com.android.art/lib64/libart.so\n");
    LOGI("/apex/com.android.art/lib64/libunwindstack.so\n");
    LOGI("/apex/com.android.runtime/lib64/bionic/libc.so\n");
    LOGI("Create Fakecore fakecore/tombstone_09.fakecore ...\n");
    LOGI("Core load (0x76dcf405bb60) \n");
    LOGI("Core env:\n");
    LOGI("  * Path: \n");
    LOGI("  * Machine: arm64\n");
    LOGI("  * Bits: 64\n");
    LOGI("  * PointSize: 8\n");
    LOGI("  * PointMask: 0xffffffffffffffff\n");
    LOGI("  * VabitsMask: 0x7fffffffff\n");
    LOGI("  * PageSize: 0x1000\n");
    LOGI("  * Remote: false\n");
    LOGI("  * Thread: 25911\n");
    LOGI("Switch android(0) env.\n");
    LOGI("New overlay [100000, 104000)\n");
    LOGI("Create FAKE PHDR\n");
    LOGI("New note overlay [29518, 55e60)\n");
    LOGI("Create FAKE DYNAMIC\n");
    LOGI("Create FAKE LINK MAP\n");
    LOGI("0x7d5ea00000 /apex/com.android.art/lib64/libart.so\n");
    LOGI("0x7d61945000 /apex/com.android.art/lib64/libunwindstack.so\n");
    LOGI("0x7e0c666000 /apex/com.android.runtime/lib64/bionic/libc.so\n");
    LOGI("Create FAKE STRTAB\n");
    LOGI("Mmap segment [7d5ea00000, 7d5eb6b000) fakecore/symbols/apex/com.android.art/lib64/libart.so [0]\n");
    LOGI("WARN: Mmap segment [7d5ea00000, 7d5eb6b000) size 16b000 != 200000, maybe reset range!\n");
    LOGI("Mmap segment [7d5ec00000, 7d5f171000) fakecore/symbols/apex/com.android.art/lib64/libart.so [200000]\n");
    LOGI("WARN: Mmap segment [7d5ec00000, 7d5f171000) size 571000 != 600000, maybe reset range!\n");
    LOGI("Read symbols[9953] (/apex/com.android.art/lib64/libart.so)\n");
    LOGI("Mmap segment [7d61945000, 7d61984000) fakecore/symbols/apex/com.android.art/lib64/libunwindstack.so [0]\n");
    LOGI("Mmap segment [7d61984000, 7d619f6000) fakecore/symbols/apex/com.android.art/lib64/libunwindstack.so [3f000]\n");
    LOGI("Read symbols[1867] (/apex/com.android.art/lib64/libunwindstack.so)\n");
    LOGI("Mmap segment [7e0c666000, 7e0c6b6000) fakecore/symbols/apex/com.android.runtime/lib64/bionic/libc.so [0]\n");
    LOGI("Mmap segment [7e0c6b6000, 7e0c771000) fakecore/symbols/apex/com.android.runtime/lib64/bionic/libc.so [50000]\n");
    LOGI("Read symbols[3211] (/apex/com.android.runtime/lib64/bionic/libc.so)\n");
    LOGI("calibrate /apex/com.android.art/lib64/libart.so l_ld(7d5f20e8f8)\n");
    LOGI("calibrate /apex/com.android.art/lib64/libunwindstack.so l_ld(7d619fcb38)\n");
    LOGI("calibrate /apex/com.android.runtime/lib64/bionic/libc.so l_ld(7e0c7762e8)\n");
    LOGI("New overlay [75452000, 95452000)\n");
    LOGI("New overlay [7b612c7000, 7b620c7000)\n");
    LOGI("New overlay [7cec46a000, 7cec46c000)\n");
    LOGI("New overlay [7d3d700000, 7d3da00000)\n");
    LOGI("New overlay [7d43600000, 7d43a00000)\n");
    LOGI("New overlay [7d45427000, 7d45522000)\n");
    LOGI("New overlay [7d61984000, 7d619f6000)\n");
    LOGI("New overlay [7d619ff000, 7d61a00000)\n");
    LOGI("New overlay [7e0c6b6000, 7e0c771000)\n");
    LOGI("FakeCore: saved [fakecore/tombstone_09.fakecore]\n");
}
