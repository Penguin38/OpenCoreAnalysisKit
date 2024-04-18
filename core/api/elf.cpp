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
#include <string>
#include <linux/elf.h>

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
    if (CoreApi::GetPointSize() == 64) {
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

void Elfx_Phdr::Init() {
    if (CoreApi::GetPointSize() == 64) {
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
    uint32_t cap = 64 / CoreApi::GetPointSize();
    __Elfx_Dynamic_offset__ = {
        .d_type = 0,
        .d_val = 8 / cap,
    };

    __Elfx_Dynamic_size__ = {
        .THIS = 16 / cap,
    };
}

void Elfx_Sym::Init() {
    if (CoreApi::GetPointSize() == 64) {
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

    Elf::Init();
    uint64_t phdr = CoreApi::FindAuxv(AT_PHDR);
    uint64_t phent = CoreApi::FindAuxv(AT_PHENT);
    uint64_t phnum = CoreApi::FindAuxv(AT_PHNUM);
    uint64_t execfn = CoreApi::FindAuxv(AT_EXECFN);

    if (!CoreApi::IsVirtualValid(phdr)) {
        std::string name;
        if (CoreApi::IsVirtualValid(execfn)) {
            name.append(reinterpret_cast<const char*>(CoreApi::GetReal(execfn)));
        }
        LOGW("WARN: Not found execfn [%s].\n", name.c_str());
        return mDebug;
    }

    Elfx_Phdr dyphdr = 0x0;
    Elfx_Phdr cphdr = phdr;
    Elfx_Ehdr ehdr(phdr - cphdr.p_vaddr(), cphdr);

    int index = 0;
    while (index < phnum) {
        if (cphdr.p_type() == PT_DYNAMIC) {
            dyphdr = cphdr;
            break;
        }
        index++;
        cphdr.MovePtr(SIZEOF(Elfx_Phdr));
    }

    mDebug = FindDynamicEntry(ehdr, dyphdr, DT_DEBUG);
    return mDebug;
}

uint64_t Elf::FindDynamicEntry(Elfx_Ehdr& ehdr, Elfx_Phdr& phdr, uint64_t type) {
    if (!phdr.Ptr()) return 0x0;

    Elfx_Dynamic dynamic = ehdr.Ptr() + phdr.p_vaddr();
    int numdynamic = phdr.p_filesz() / SIZEOF(Elfx_Dynamic);
    int index = 0;
    while (index < numdynamic) {
        if (dynamic.d_type() == type) {
            return dynamic.d_val();
        }
        index++;
        dynamic.MovePtr(SIZEOF(Elfx_Dynamic));
    }

    return 0x0;
}

uint64_t Elf::DynamicSymbol(LinkMap* handle, const char* symbol) {
    Elfx_Ehdr ehdr(handle->begin(), handle->block());
    Elfx_Phdr phdr(ehdr.Ptr() + ehdr.e_phoff(), ehdr);
    int phnum = ehdr.e_phnum();

    Elfx_Phdr dyphdr = 0x0;
    int index = 0;

    while (index < phnum) {
        if (phdr.p_type() == PT_DYNAMIC) {
            dyphdr = phdr;
            break;
        }
        index++;
        phdr.MovePtr(SIZEOF(Elfx_Phdr));
    }

    uint64_t strtab = FindDynamicEntry(ehdr, dyphdr, DT_STRTAB);
    uint64_t symtab = FindDynamicEntry(ehdr, dyphdr, DT_SYMTAB);
    uint64_t syment = FindDynamicEntry(ehdr, dyphdr, DT_SYMENT);
    uint64_t versym = FindDynamicEntry(ehdr, dyphdr, DT_VERSYM);

    // check page fault
    if (!syment) return 0;

    // maybe error match
    uint64_t symsz = strtab > versym ? (versym - symtab) : (strtab - symtab);
    int64_t count = symsz / syment;

    api::MemoryRef tables(ehdr.Ptr() + strtab, ehdr);
    Elfx_Sym symbols(ehdr.Ptr() + symtab, ehdr);

    for (int i = 0; i < count; ++i) {
        std::string name = reinterpret_cast<const char* >(tables.Real() + symbols.st_name());
        if (name == symbol) {
            return symbols.st_value();
        }
        symbols.MovePtr(syment);
    }
    return 0;
}

} // namespace api
