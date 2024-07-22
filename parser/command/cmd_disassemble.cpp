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
#include "base/utils.h"
#include "command/cmd_disassemble.h"
#include "common/disassemble/capstone.h"
#include "common/exception.h"
#include "common/elf.h"
#include <linux/elf.h>
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>
#include <cxxabi.h>

int DisassembleCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || argc < 2)
        return 0;

    int read_opt = OPT_READ_ALL;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"origin",  no_argument,       0,  0 },
        {"mmap",    no_argument,       0,  1 },
        {"overlay", no_argument,       0,  2 },
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "012",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                read_opt = OPT_READ_OR;
                break;
            case 1:
                read_opt = OPT_READ_MMAP;
                break;
            case 2:
                read_opt = OPT_READ_OVERLAY;
                break;
        }
    }

    if (optind >= argc) {
        return 0;
    }

    const char* symbol = argv[optind];
    auto callback = [&](LinkMap* map) -> bool {
        SymbolEntry entry = map->DlSymEntry(symbol);
        if (entry.IsValid()) {
            LOGI("LIB: " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, map->name());

            std::string d_symbol;
            int status;
            char* demangled_name = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
            if (status == 0) {
                d_symbol = demangled_name;
                std::free(demangled_name);
            } else {
                d_symbol = symbol;
            }

            bool vdso = !strcmp(map->name(), "[vdso]");
            uint64_t vaddr = map->l_addr() + entry.offset;
            if (ELF_ST_TYPE(entry.type) == STT_FUNC
                    || (vdso && ELF_ST_TYPE(entry.type) == STT_NOTYPE)) {
                bool thumb = false;
                if (CoreApi::GetMachine() == EM_ARM) {
                    if (entry.offset & 0x1) {
                        vaddr &= (CoreApi::GetPointMask() - 1);
                        thumb = true;
                    }
                }

                capstone::Disassember::Option opt(vaddr, -1);
                if (CoreApi::GetMachine() == EM_ARM) {
                    opt.SetArchMode(capstone::Disassember::Option::ARCH_ARM, thumb?
                                    capstone::Disassember::Option::MODE_THUMB : capstone::Disassember::Option::MODE_ARM);
                }

                uint8_t* data = reinterpret_cast<uint8_t*>(CoreApi::GetReal(vaddr, read_opt));
                if (data) {
                    LOGI(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET ":\n", d_symbol.c_str());
                    capstone::Disassember::Dump("  ", data, entry.size, vaddr, opt);
                }
            } else {
                LOGI("  * %s: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, d_symbol.c_str(), vaddr);
            }
            return true;
        }
        return false;
    };
    CoreApi::ForeachLinkMap(callback);
    return 0;
}

void DisassembleCommand::usage() {
    LOGI("Usage: disassemble|disas <SYM> [Option...]\n");
}
