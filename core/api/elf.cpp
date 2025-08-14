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
#include "api/elf.h"
#include "api/core.h"
#include "common/auxv.h"
#include "common/elf.h"
#include <linux/elf.h>
#include <string.h>
#include <cxxabi.h>
#include <string>
#include <limits>

struct Elfx_Ehdr_OffsetTable __Elfx_Ehdr_offset__;
struct Elfx_Ehdr_SizeTable __Elfx_Ehdr_size__;
struct Elfx_Phdr_OffsetTable __Elfx_Phdr_offset__;
struct Elfx_Phdr_SizeTable __Elfx_Phdr_size__;
struct Elfx_Dynamic_OffsetTable __Elfx_Dynamic_offset__;
struct Elfx_Dynamic_SizeTable __Elfx_Dynamic_size__;
struct Elfx_Sym_OffsetTable __Elfx_Sym_offset__;
struct Elfx_Sym_SizeTable __Elfx_Sym_size__;

namespace api {

void Elfx_Ehdr::Init() {
    if (CoreApi::Bits() == 64) {
        __Elfx_Ehdr_offset__ = {
            .e_ident = 0,
            .e_type = 16,
            .e_machine = 18,
            .e_version = 20,
            .e_entry = 24,
            .e_phoff = 32,
            .e_shoff = 40,
            .e_flags = 48,
            .e_ehsize = 52,
            .e_phentsize = 54,
            .e_phnum = 56,
            .e_shentsize = 58,
            .e_shnum = 60,
            .e_shstrndx = 62,
        };

        __Elfx_Ehdr_size__ = {
            .THIS = 64,
        };
    } else {
        __Elfx_Ehdr_offset__ = {
            .e_ident = 0,
            .e_type = 16,
            .e_machine = 18,
            .e_version = 20,
            .e_entry = 24,
            .e_phoff = 28,
            .e_shoff = 32,
            .e_flags = 36,
            .e_ehsize = 40,
            .e_phentsize = 42,
            .e_phnum = 44,
            .e_shentsize = 46,
            .e_shnum = 48,
            .e_shstrndx = 50,
        };

        __Elfx_Ehdr_size__ = {
            .THIS = 52,
        };
    }
}

bool Elfx_Ehdr::IsElf() {
    if (IsValid()) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(Real());
        return !memcmp(header->ident, ELFMAG, 4);
    }
    return false;
}

void Elfx_Phdr::Init() {
    if (CoreApi::Bits() == 64) {
        __Elfx_Phdr_offset__ = {
            .p_type = 0,
            .p_flags = 4,
            .p_offset = 8,
            .p_vaddr = 16,
            .p_paddr = 24,
            .p_filesz = 32,
            .p_memsz = 40,
            .p_align = 48,
        };

        __Elfx_Phdr_size__ = {
            .THIS = 56,
        };
    } else {
        __Elfx_Phdr_offset__ = {
            .p_type = 0,
            .p_flags = 4,
            .p_offset = 8,
            .p_vaddr = 12,
            .p_paddr = 16,
            .p_filesz = 20,
            .p_memsz = 24,
            .p_align = 28,
        };

        __Elfx_Phdr_size__ = {
            .THIS = 32,
        };
    }
}

void Elfx_Dynamic::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __Elfx_Dynamic_offset__ = {
        .d_type = 0,
        .d_val = 8 / cap,
    };

    __Elfx_Dynamic_size__ = {
        .THIS = 16 / cap,
    };
}

void Elfx_Sym::Init() {
    if (CoreApi::Bits() == 64) {
        __Elfx_Sym_offset__ = {
            .st_name = 0,
            .st_value = 8,
            .st_size = 16,
            .st_info = 4,
            .st_other = 5,
            .st_shndx = 6,
        };

        __Elfx_Sym_size__ = {
            .THIS = 24,
        };
    } else {
        __Elfx_Sym_offset__ = {
            .st_name = 0,
            .st_value = 4,
            .st_size = 8,
            .st_info = 12,
            .st_other = 13,
            .st_shndx = 14,
        };

        __Elfx_Sym_size__ = {
            .THIS = 16,
        };
    }
}

void Elf::Init() {
    Elfx_Ehdr::Init();
    Elfx_Phdr::Init();
    Elfx_Dynamic::Init();
    Elfx_Sym::Init();
}

MemoryRef& Elf::GetDebug() {
    if (mDebug.Ptr())
        return mDebug;

    Elfx_Phdr phdr = CoreApi::FindAuxv(AT_PHDR);
    uint64_t phent = CoreApi::FindAuxv(AT_PHENT);
    uint64_t phnum = CoreApi::FindAuxv(AT_PHNUM);
    uint64_t execfn = CoreApi::FindAuxv(AT_EXECFN);

    if (!phdr.IsValid()) {
        std::string name;
        if (CoreApi::IsVirtualValid(execfn)) {
            name.append(reinterpret_cast<const char*>(CoreApi::GetReal(execfn)));
        }
        LOGW("Not found exec [%s].\n", name.c_str());
        return mDebug;
    }

    Elfx_Dynamic dynamic = 0x0;
    Elfx_Phdr tmp = phdr;
    int index = 0;
    while (index < phnum) {
        if (tmp.p_type() == PT_DYNAMIC) {
            dynamic = phdr.Ptr() - phdr.p_vaddr() + tmp.p_vaddr();
            break;
        }
        index++;
        tmp.MovePtr(SIZEOF(Elfx_Phdr));
    }

    if (dynamic.IsValid()) {
        mDebug = FindDynamicEntry(dynamic, DT_DEBUG);
    } else {
        std::string name;
        if (CoreApi::IsVirtualValid(execfn)) {
            name.append(reinterpret_cast<const char*>(CoreApi::GetReal(execfn)));
        }
        LOGW("Not found exec dynamic [%s].\n", name.c_str());
    }
    return mDebug;
}

uint64_t Elf::FindDynamicEntry(Elfx_Dynamic& dynamic, uint64_t type) {
    if (!dynamic.IsValid()) return 0x0;
    uint64_t value = 0x0;
    Elfx_Dynamic tmp = dynamic;
    do {
        if (tmp.d_type() == type) {
            value = tmp.d_val();
            break;
        }
        tmp.MovePtr(SIZEOF(Elfx_Dynamic));
    } while(tmp.d_type() != 0x0);
    return value;
}

Elfx_Dynamic Elf::FindDynamic(LinkMap* handle) {
    Elfx_Dynamic dynamic = 0x0;

    if (handle->l_ld()) {
        dynamic = handle->l_ld();
    } else {
        Elfx_Ehdr ehdr(handle->l_addr());
        if (!ehdr.IsElf()) {
            LOGD("%s maybe relocation!\n", handle->name());
            return dynamic;
        }

        Elfx_Phdr phdr(ehdr.Ptr() + SIZEOF(Elfx_Ehdr), ehdr);
        int phnum = ehdr.e_phnum();

        Elfx_Phdr tmp = phdr;
        int index = 0;
        while (index < phnum) {
            if (tmp.p_type() == PT_PHDR) {
                if (tmp.p_offset() != tmp.p_vaddr()) {
                    LOGD("%s relocation!\n", handle->name());
                    break;
                }
            }
            if (tmp.p_type() == PT_DYNAMIC) {
                dynamic = handle->l_addr() + tmp.p_vaddr();
                break;
            }
            index++;
            tmp.MovePtr(SIZEOF(Elfx_Phdr));
        }
    }
    return dynamic;
}

void Elf::ReadSymbols(LinkMap* handle) {
    if (!handle->l_addr()) return;
    Elfx_Dynamic dynamic = FindDynamic(handle);

    uint64_t strtab = FindDynamicEntry(dynamic, DT_STRTAB);
    uint64_t symtab = FindDynamicEntry(dynamic, DT_SYMTAB);
    uint64_t syment = FindDynamicEntry(dynamic, DT_SYMENT);
    uint64_t versym = FindDynamicEntry(dynamic, DT_VERSYM);

    // check page fault
    if (!syment && syment != SIZEOF(Elfx_Sym)) return;

    // maybe error match
    int64_t symsz = strtab > versym ? (versym - symtab) : (strtab - symtab);
    if (symsz <= 0 || symsz % syment)
        return;

    int64_t count = symsz / syment;
    api::MemoryRef tables = 0x0;
    Elfx_Sym symbols = 0x0;

    if (handle->l_addr() <= strtab)
        tables = strtab;
    else
        tables = handle->l_addr() + strtab;

    if (handle->l_addr() <= symtab)
        symbols = symtab;
    else
        symbols = handle->l_addr() + symtab;

    if (!tables.IsValid() || !symbols.IsValid())
        return;

    std::unordered_set<SymbolEntry, SymbolEntry::Hash>& dynsyms = handle->GetDynsyms();
    for (int i = 0; i < count; ++i) {
        if (symbols.st_value() && symbols.st_size()) {
            api::MemoryRef symname = tables.Ptr() + symbols.st_name();
            if (symname.Ptr() >= CoreApi::GetPointMask())
                continue;

            if (!tables.Block()->virtualContains(symname.Ptr()))
                break;

            SymbolEntry entry = SymbolEntry(symbols.st_value(), symbols.st_info(), symbols.st_size(),
                                            reinterpret_cast<const char* >(symname.Real()));
            dynsyms.insert(entry);
        }
        symbols.MovePtr(syment);
    }
}

} // namespace api
