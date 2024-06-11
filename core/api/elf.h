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

#ifndef CORE_API_ELF_H_
#define CORE_API_ELF_H_

#include "api/memory_ref.h"
#include "common/link_map.h"

struct Elfx_Ehdr_OffsetTable {
    uint32_t e_ident;
    uint32_t e_type;
    uint32_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint32_t e_ehsize;
    uint32_t e_phentsize;
    uint32_t e_phnum;
    uint32_t e_shentsize;
    uint32_t e_shnum;
    uint32_t e_shstrndx;
};

struct Elfx_Ehdr_SizeTable {
    uint32_t THIS;
};

extern struct Elfx_Ehdr_OffsetTable __Elfx_Ehdr_offset__;
extern struct Elfx_Ehdr_SizeTable __Elfx_Ehdr_size__;

struct Elfx_Phdr_OffsetTable {
    uint32_t p_type;
    uint32_t p_flags;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_align;
};

struct Elfx_Phdr_SizeTable {
    uint32_t THIS;
};

extern struct Elfx_Phdr_OffsetTable __Elfx_Phdr_offset__;
extern struct Elfx_Phdr_SizeTable __Elfx_Phdr_size__;

struct Elfx_Dynamic_OffsetTable {
    uint32_t d_type;
    uint32_t d_val;
};

struct Elfx_Dynamic_SizeTable {
    uint32_t THIS;
};

extern struct Elfx_Dynamic_OffsetTable __Elfx_Dynamic_offset__;
extern struct Elfx_Dynamic_SizeTable __Elfx_Dynamic_size__;

struct Elfx_Sym_OffsetTable {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    uint32_t st_info;
    uint32_t st_other;
    uint32_t st_shndx;
};

struct Elfx_Sym_SizeTable {
    uint32_t THIS;
};

extern struct Elfx_Sym_OffsetTable __Elfx_Sym_offset__;
extern struct Elfx_Sym_SizeTable __Elfx_Sym_size__;

namespace api {

class Elfx_Ehdr : public MemoryRef {
public:
    Elfx_Ehdr(uint64_t v) : MemoryRef(v) {}
    Elfx_Ehdr(uint64_t v, LoadBlock* b) : MemoryRef(v, b) {}
    Elfx_Ehdr(const MemoryRef& ref) : MemoryRef(ref) {}
    Elfx_Ehdr(uint64_t v, MemoryRef& ref) : MemoryRef(v, ref) {}
    Elfx_Ehdr(uint64_t v, MemoryRef* ref) : MemoryRef(v, ref) {}

    inline bool operator==(Elfx_Ehdr& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Elfx_Ehdr& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t e_phoff() { return VALUEOF(Elfx_Ehdr, e_phoff); }
    inline uint16_t e_phnum() { return *reinterpret_cast<uint16_t *>(Real() + OFFSET(Elfx_Ehdr, e_phnum)); }
};

class Elfx_Phdr : public MemoryRef {
public:
    Elfx_Phdr(uint64_t v) : MemoryRef(v) {}
    Elfx_Phdr(const MemoryRef& ref) : MemoryRef(ref) {}
    Elfx_Phdr(uint64_t v, MemoryRef& ref) : MemoryRef(v, ref) {}
    Elfx_Phdr(uint64_t v, MemoryRef* ref) : MemoryRef(v, ref) {}

    inline bool operator==(Elfx_Phdr& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Elfx_Phdr& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint32_t p_type() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Elfx_Phdr, p_type)); }
    inline uint64_t p_vaddr() { return VALUEOF(Elfx_Phdr, p_vaddr); }
    inline uint64_t p_filesz() { return VALUEOF(Elfx_Phdr, p_filesz); }
    inline uint64_t p_offset() { return VALUEOF(Elfx_Phdr, p_offset); }
};

class Elfx_Dynamic : public MemoryRef {
public:
    Elfx_Dynamic(uint64_t v) : MemoryRef(v) {}
    Elfx_Dynamic(const MemoryRef& ref) : MemoryRef(ref) {}
    Elfx_Dynamic(uint64_t v, MemoryRef& ref) : MemoryRef(v, ref) {}
    Elfx_Dynamic(uint64_t v, MemoryRef* ref) : MemoryRef(v, ref) {}

    inline bool operator==(Elfx_Dynamic& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Elfx_Dynamic& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t d_type() { return VALUEOF(Elfx_Dynamic, d_type); }
    inline uint64_t d_val() { return VALUEOF(Elfx_Dynamic, d_val); }
};

class Elfx_Sym : public MemoryRef {
public:
    Elfx_Sym(uint64_t v) : MemoryRef(v) {}
    Elfx_Sym(const MemoryRef& ref) : MemoryRef(ref) {}
    Elfx_Sym(uint64_t v, MemoryRef& ref) : MemoryRef(v, ref) {}
    Elfx_Sym(uint64_t v, MemoryRef* ref) : MemoryRef(v, ref) {}

    inline bool operator==(Elfx_Sym& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Elfx_Sym& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline unsigned char st_info() { return *reinterpret_cast<unsigned char *>(Real() + OFFSET(Elfx_Sym, st_info)); }
    inline uint32_t st_name() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Elfx_Sym, st_name)); }
    inline uint64_t st_value() { return VALUEOF(Elfx_Sym, st_value); }
    inline uint64_t st_size() { return VALUEOF(Elfx_Sym, st_size); }
};

class Elf {
public:
    Elf() {}
    MemoryRef& GetDebug();
    uint64_t FindDynamicEntry(Elfx_Dynamic& dynamic, uint64_t type);
    uint64_t DynamicSymbol(LinkMap* handle, const char* symbol);

    static void Init();
private:
    MemoryRef mDebug;
};

} // namespace api
#endif  // CORE_API_ELF_H_
