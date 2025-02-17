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
#include "lp64/core.h"
#include "lp32/core.h"
#include "api/elf.h"
#include "common/link_map.h"
#include "common/exception.h"
#include "common/load_block.h"
#include "common/elf.h"
#include <linux/elf.h>
#include <cxxabi.h>

struct LinkMap_OffsetTable __LinkMap_offset__;
struct LinkMap_SizeTable __LinkMap_size__;

void LinkMap::Init() {
    uint32_t cap = 64 / CoreApi::Bits();
    __LinkMap_offset__ = {
        .l_addr = 0,
        .l_name = 8 / cap,
        .l_ld = 16 / cap,
        .l_next = 24 / cap,
        .l_prev = 32 / cap,
    };

    __LinkMap_size__ = {
        .THIS = 40 / cap,
    };
}

api::MemoryRef& LinkMap::GetAddrCache() {
    if (addr_cache.Ptr())
        return addr_cache;

    try {
        api::Elfx_Dynamic dynamic = api::Elf::FindDynamic(this);
        uint64_t symtab = api::Elf::FindDynamicEntry(dynamic, DT_SYMTAB);

        api::Elfx_Sym symbols = 0x0;
        if (l_addr() <= symtab) symbols = symtab;
        else symbols = l_addr() + symtab;

        if (symbols.IsValid()) {
            api::Elfx_Ehdr ehdr(symbols.Block()->vaddr());
            if (ehdr.IsElf()) addr_cache = symbols.Block()->vaddr();
        }
    } catch(InvalidAddressException& e) {
        return addr_cache;
    }

    addr_cache.Prepare(false);
    return addr_cache;
}

api::MemoryRef& LinkMap::GetNameCache() {
    if (!name_cache.Ptr()) {
        name_cache = l_name();
        name_cache.Prepare(false);
    }
    return name_cache;
}

uint64_t LinkMap::begin() {
    return GetAddrCache().Ptr();
}

const char* LinkMap::name() {
    if (!GetNameCache().IsValid() || !GetNameCache().value8Of()) {
        File* dynamic = CoreApi::FindFile(l_ld());
        return dynamic ? dynamic->name().data() : "";
    }
    return reinterpret_cast<const char*>(GetNameCache().Real());
}

LoadBlock* LinkMap::block() {
    GetAddrCache().Prepare(false);
    return GetAddrCache().Block();
}

void LinkMap::NiceMethod(uint64_t pc, NiceSymbol& symbol) {
    uint64_t nice_offset = 0;
    uint64_t nice_size = 0;

    SymbolEntry entry = DlRegionSymEntry(pc);
    if (!entry.IsValid())
        return;

    bool vdso = !strcmp(name(), "[vdso]") || l_addr() == CoreApi::FindAuxv(AT_SYSINFO_EHDR);
    if (ELF_ST_TYPE(entry.type) == STT_FUNC
            || (vdso && ELF_ST_TYPE(entry.type) == STT_NOTYPE)) {
        nice_offset = entry.offset + l_addr();
        if (CoreApi::GetMachine() == EM_ARM)
            nice_offset &= (CoreApi::GetPointMask() - 1);
        nice_size = entry.size;
        symbol.SetNiceMethod(entry.symbol.data(), nice_offset, nice_size);
    }
}

SymbolEntry LinkMap::DlSymEntry(const char* symbol) {
    std::unordered_set<SymbolEntry, SymbolEntry::Hash>& symbols = GetCurrentSymbols();
    const auto& it = std::find_if(symbols.begin(), symbols.end(),
            [&](const SymbolEntry& entry) {
                return entry.symbol == symbol;
            });
    if (it != symbols.end())
        return *it;
    return SymbolEntry::Invalid();
}

SymbolEntry LinkMap::DlRegionSymEntry(uint64_t addr) {
    std::unordered_set<SymbolEntry, SymbolEntry::Hash>& symbols = GetCurrentSymbols();
    if (!symbols.size())
        return SymbolEntry::Invalid();

    uint64_t cloc_addr = addr & CoreApi::GetVabitsMask();
    if (cloc_addr <= l_addr())
        return SymbolEntry::Invalid();

    uint64_t cloc_offset = cloc_addr - l_addr();
    const auto& it = std::find_if(symbols.begin(), symbols.end(),
            [&](const SymbolEntry& entry) {
                uint64_t offset = entry.offset;
                if (CoreApi::GetMachine() == EM_ARM)
                    offset &= (CoreApi::GetPointMask() - 1);

                if (offset > cloc_offset)
                    return false;

                if (cloc_offset < offset + entry.size)
                    return true;

                return false;
            });
    if (it != symbols.end())
        return *it;
    return SymbolEntry::Invalid();
}

void LinkMap::ReadSymbols() {
    LoadBlock* load = block();
    if (load && load->isMmapBlock()) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(load->begin());
        if (!header->CheckLibrary(load->name().c_str()))
            return;

        std::unordered_set<SymbolEntry, SymbolEntry::Hash>& symbols = load->GetSymbols();
        symbols.clear(); // clear prev symbols
        if (CoreApi::Bits() == 64) {
            lp64::Core::readsym64(this);
        } else {
            lp32::Core::readsym32(this);
        }
        if (symbols.size()) LOGI(ANSI_COLOR_GREEN "Read symbols[%ld] (%s)\n" ANSI_COLOR_RESET, symbols.size(), name());
    }
}

void LinkMap::ReadDynsyms() {
    dynsyms.clear();
    try {
        api::Elf::ReadSymbols(this);
    } catch(InvalidAddressException& e) {
    }
    if (dynsyms.size()) LOGD("Read dynsyms[%ld] (%s)\n", dynsyms.size(), name());
}

std::unordered_set<SymbolEntry, SymbolEntry::Hash>& LinkMap::GetCurrentSymbols() {
    LoadBlock* load = block();
    if (load && load->isMmapBlock()) {
        return load->GetSymbols();
    } else {
        return GetDynsyms();
    }
}

std::string& LinkMap::NiceSymbol::GetMethod() {
    if (method.length() == 0) {
        int status;
        char* demangled_name = abi::__cxa_demangle(symbol.data(), nullptr, nullptr, &status);
        if (status == 0) {
            method = demangled_name;
            std::free(demangled_name);
        } else {
            method = symbol;
        }
    }
    return method;
}
