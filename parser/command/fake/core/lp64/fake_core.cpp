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
#include "lp64/core.h"
#include "common/bit.h"
#include "common/elf.h"
#include "common/auxv.h"
#include "common/exception.h"
#include "command/fake/core/lp64/fake_core.h"
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <linux/elf.h>

namespace lp64 {

int FakeCore::execute(const char* output) {
    if (!CoreApi::IsReady())
        return -1;

    FILE *fp = fopen(output, "wb");
    if (!fp) {
        LOGE("Can't open \"%s\".\n", output);
        return -1;
    }

    // Write ELF Header
    Elf64_Ehdr ehdr;
    memcpy(&ehdr, (void *)CoreApi::GetBegin(), sizeof(Elf64_Ehdr));
    ehdr.e_phnum = CoreApi::GetLoads(false).size() + CoreApi::GetNotes().size();
    fwrite(&ehdr, sizeof(Elf64_Ehdr), 1, fp);

    // Write Program Headers
    uint64_t current_offset = sizeof(Elf64_Ehdr) + ehdr.e_phnum * sizeof(Elf64_Phdr);;
    Elf64_Phdr *tmp = reinterpret_cast<Elf64_Phdr *>(malloc(sizeof(Elf64_Phdr) * ehdr.e_phnum));

    int num = 0;
    // PT_NOTE
    std::vector<std::unique_ptr<NoteBlock>>& notes = CoreApi::GetNotes();
    for (const auto& note : notes) {
        tmp[num].p_type = PT_NOTE;
        tmp[num].p_flags = note->flags();
        tmp[num].p_offset = current_offset;
        tmp[num].p_vaddr = note->vaddr();
        tmp[num].p_paddr = note->paddr();
        tmp[num].p_filesz = note->realSize();
        tmp[num].p_memsz = note->memsz();
        tmp[num].p_align = note->align();

        current_offset = RoundUp(tmp[num].p_offset + tmp[num].p_filesz, ELF_PAGE_SIZE);
        fwrite(&tmp[num], sizeof(Elf64_Phdr), 1, fp);
        ++num;
    }

    // PT_LOAD
    std::vector<std::shared_ptr<LoadBlock>>& loads = CoreApi::GetLoads(false);
    for (const auto& block : loads) {
        tmp[num].p_type = PT_LOAD;
        tmp[num].p_flags = block->flags();
        tmp[num].p_offset = current_offset;
        tmp[num].p_vaddr = block->vaddr();
        tmp[num].p_paddr = block->paddr();
        tmp[num].p_filesz = block->realSize();
        tmp[num].p_memsz = block->memsz();
        tmp[num].p_align = block->align();

        if (!block->realSize() && block->isValid())
            tmp[num].p_filesz = tmp[num].p_memsz;

        current_offset += tmp[num].p_filesz;
        fwrite(&tmp[num], sizeof(Elf64_Phdr), 1, fp);
        ++num;
    }

    uint64_t current_filesz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * ehdr.e_phnum;
    // Write Segment
    uint8_t* zero_buf = (uint8_t*)malloc(ELF_PAGE_SIZE);
    memset(zero_buf, 0x0, ELF_PAGE_SIZE);

    // reset num
    num = 0;
    for (const auto& note : notes) {
        fwrite(reinterpret_cast<void *>(note->begin()), tmp[num].p_filesz, 1, fp);
        current_filesz += tmp[num].p_filesz;
        if (!IS_ALIGNED(current_filesz, ELF_PAGE_SIZE)) {
            uint64_t aliged_size = RoundUp(current_filesz, ELF_PAGE_SIZE) - current_filesz;
            fwrite(zero_buf, aliged_size, 1, fp);
            current_filesz += aliged_size;
        }
        ++num;
    }

    for (const auto& block : loads) {
        if (!tmp[num].p_filesz) {
            ++num;
            continue;
        }

        if (block->isValid()) {
            current_filesz += tmp[num].p_filesz;
            fwrite(reinterpret_cast<void *>(block->begin()), tmp[num].p_filesz, 1, fp);
        }
        ++num;
    }

    free(zero_buf);
    free(tmp);
    fclose(fp);
    LOGI("FakeCore: saved [%s]\n", output);
    return 0;
}

void FakeCore::ParseProcessMapsVma(std::vector<Opencore::VirtualMemoryArea>& maps) {
    if (!maps.size())
        return;

    phnum = maps.size();
    phdr = (Elf64_Phdr *)malloc(phnum * sizeof(Elf64_Phdr));
    memset(phdr, 0, phnum * sizeof(Elf64_Phdr));
    file = (lp64::File *)malloc(phnum * sizeof(lp64::File));
    memset(file, 0, phnum * sizeof(lp64::File));

    for (int index = 0; index < maps.size(); ++index) {
        Opencore::VirtualMemoryArea& vma = maps[index];
        ParserPhdr(index, vma);
        ParserNtFile(index, vma);
    }
}

void FakeCore::ParserPhdr(int index, Opencore::VirtualMemoryArea& vma) {
    phdr[index].p_type = PT_LOAD;

    phdr[index].p_vaddr = (Elf64_Addr)vma.begin;
    phdr[index].p_paddr = 0x0;
    phdr[index].p_memsz = (Elf64_Addr)vma.end-(Elf64_Addr)vma.begin;

    if (vma.flags[0] == 'r' || vma.flags[0] == 'R')
        phdr[index].p_flags = phdr[index].p_flags | PF_R;

    if (vma.flags[1] == 'w' || vma.flags[1] == 'W')
        phdr[index].p_flags = phdr[index].p_flags | PF_W;

    if (vma.flags[2] == 'x' || vma.flags[2] == 'X')
        phdr[index].p_flags = phdr[index].p_flags | PF_X;

    phdr[index].p_filesz = 0x0;
    phdr[index].p_align = align_size;
}

void FakeCore::ParserNtFile(int index, Opencore::VirtualMemoryArea& vma) {
    file[index].begin = (Elf64_Addr)vma.begin;
    file[index].end = (Elf64_Addr)vma.end;
    file[index].offset = vma.offset >> 12;
    fileslen += vma.file.length() + 1;
}

void FakeCore::CreateCoreHeader() {
    snprintf((char *)ehdr.e_ident, 5, ELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_CORE;
    ehdr.e_machine = getMachine();
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0x0;
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_shoff = 0x0;
    ehdr.e_flags = 0x0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = phnum + 1;
    ehdr.e_shentsize = 0x0;
    ehdr.e_shnum = 0x0;
    ehdr.e_shstrndx = 0x0;
}

void FakeCore::CreateCoreNoteHeader() {
    note.p_type = PT_NOTE;
    note.p_offset = sizeof(Elf64_Ehdr) + ehdr.e_phnum * sizeof(Elf64_Phdr);
}

void FakeCore::CreateCoreAUXV() {
    auxvnum = DEF_FAKE_AUXV_NUM;
    auxv = (lp64::Auxv *)malloc(auxvnum * sizeof(lp64::Auxv));
    memset(auxv, 0, auxvnum * sizeof(lp64::Auxv));

    auxv[use_auxv_num++].init(AT_PHDR, 0x0);
    auxv[use_auxv_num++].init(AT_PHENT, sizeof(Elf64_Phdr));
    auxv[use_auxv_num++].init(AT_PHNUM, DEF_FAKE_PHNUM);
    auxv[use_auxv_num++].init(AT_PAGESZ, page_size);
}

void FakeCore::ClocNoteFileSize() {
    note.p_filesz += sizeof(lp64::Auxv) * auxvnum + sizeof(Elf64_Nhdr) + 8;
    note.p_filesz += extra_note_filesz;
    note.p_filesz += sizeof(lp64::File) * phnum + sizeof(Elf64_Nhdr) + 8 + 2 * 8 + RoundUp(fileslen, 4);
}

uint64_t FakeCore::WriteCoreHeader(std::unique_ptr<MemoryMap>& map, uint64_t off) {
    uint64_t mem_size = sizeof(Elf64_Ehdr);
    memcpy(reinterpret_cast<void *>(map->data() + off), (void *)&ehdr, mem_size);
    return mem_size;
}

uint64_t FakeCore::WriteCoreNoteHeader(std::unique_ptr<MemoryMap>& map, uint64_t off) {
    uint64_t mem_size = sizeof(Elf64_Phdr);
    memcpy(reinterpret_cast<void *>(map->data() + off), (void *)&note, mem_size);
    return mem_size;
}

uint64_t FakeCore::WriteCoreProgramHeaders(std::unique_ptr<MemoryMap>& map, uint64_t off) {
    if (!phnum) return 0;

    uint64_t tmp_off = 0x0;
    uint64_t offset = RoundUp(note.p_offset + note.p_filesz, align_size);
    phdr[0].p_offset = offset;
    memcpy(reinterpret_cast<void *>(map->data() + off), (void *)&phdr[0], sizeof(Elf64_Phdr));
    tmp_off += sizeof(Elf64_Phdr);

    int index = 1;
    while (index < phnum) {
        phdr[index].p_offset = phdr[index - 1].p_offset + phdr[index-1].p_filesz;
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&phdr[index], sizeof(Elf64_Phdr));
        tmp_off += sizeof(Elf64_Phdr);
        index++;
    }
    return tmp_off;
}

uint64_t FakeCore::WriteCoreAUXV(std::unique_ptr<MemoryMap>& map, uint64_t off) {
    uint64_t tmp_off = 0x0;

    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(lp64::Auxv) * auxvnum;
    elf_nhdr.n_type = NT_AUXV;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&elf_nhdr, sizeof(Elf64_Nhdr));
    tmp_off += sizeof(Elf64_Nhdr);
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)magic, sizeof(magic));
    tmp_off += sizeof(magic);

    int index = 0;
    while (index < auxvnum) {
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&auxv[index], sizeof(lp64::Auxv));
        tmp_off += sizeof(lp64::Auxv);
        index++;
    }

    return tmp_off;
}

uint64_t FakeCore::WriteNtFile(std::unique_ptr<MemoryMap>& map, uint64_t off, std::vector<Opencore::VirtualMemoryArea>& maps) {
    uint64_t tmp_off = 0x0;

    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(lp64::File) * phnum + 2 * 8 + RoundUp(fileslen, 4);
    elf_nhdr.n_type = NT_FILE;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&elf_nhdr, sizeof(Elf64_Nhdr));
    tmp_off += sizeof(Elf64_Nhdr);
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)magic, sizeof(magic));
    tmp_off += sizeof(magic);

    uint64_t number = phnum;
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&number, sizeof(uint64_t));
    tmp_off += sizeof(uint64_t);
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&page_size, sizeof(uint64_t));
    tmp_off += sizeof(uint64_t);

    for (int index = 0; index < phnum; ++index) {
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&file[index], sizeof(lp64::File));
        tmp_off += sizeof(lp64::File);
    }

    for (int index = 0; index < phnum; ++index) {
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)maps[index].file.data(), maps[index].file.length() + 1);
        tmp_off += maps[index].file.length() + 1;
    }

    return tmp_off;
}

void FakeCore::CreateFakePhdr(uint64_t fake_phdr, uint64_t fake_dynamic) {
    LOGI("Create FAKE PHDR\n");
    auto fake_phdr_callback = [&](::Auxv* auxv) -> bool {
        if (auxv->type() == AT_PHDR) {
            auxv->setValue(fake_phdr + sizeof(Elf64_Ehdr));
        } else if (auxv->type() == AT_EXECFN) {
            auxv->setValue(0x0);
        }
        return false;
    };
    CoreApi::ForeachAuxv(fake_phdr_callback);

    Elf64_Ehdr ehdr;
    snprintf((char *)ehdr.e_ident, 5, ELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_DYN;
    ehdr.e_machine = CoreApi::GetMachine();
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0x0;
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_shoff = 0x0;
    ehdr.e_flags = 0x0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = DEF_FAKE_PHNUM;
    ehdr.e_shentsize = 0x0;
    ehdr.e_shnum = 0x0;
    ehdr.e_shstrndx = 0x0;
    CoreApi::Write(fake_phdr, (void *)&ehdr, sizeof(Elf64_Ehdr));

    Elf64_Phdr tmp;
    tmp.p_type = PT_PHDR;
    tmp.p_flags = PF_R;
    tmp.p_offset = sizeof(Elf64_Ehdr);
    tmp.p_vaddr = sizeof(Elf64_Ehdr);
    tmp.p_paddr = sizeof(Elf64_Ehdr);
    tmp.p_filesz = 0x0;
    tmp.p_memsz = CoreApi::GetPageSize();
    tmp.p_align = 0x8;
    CoreApi::Write(fake_phdr + sizeof(Elf64_Ehdr), (void *)&tmp, sizeof(Elf64_Phdr));

    tmp.p_type = PT_DYNAMIC;
    tmp.p_flags = PF_R | PF_W;
    tmp.p_offset = CoreApi::GetPageSize();
    tmp.p_vaddr = CoreApi::GetPageSize() * FAKE_PHDR_PAGES;
    tmp.p_paddr = tmp.p_vaddr;
    tmp.p_filesz = CoreApi::GetPageSize() * FKAE_DYNAMIC_PAGES;
    tmp.p_memsz = tmp.p_filesz;
    tmp.p_align = 0x8;
    CoreApi::Write(fake_phdr + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr), (void *)&tmp, sizeof(Elf64_Phdr));
}

void FakeCore::CreateFakeDynamic(uint64_t fake_dynamic, uint64_t fake_link_map) {
    LOGI("Create FAKE DYNAMIC\n");
    lp64::Dynamic dynamic;
    dynamic.type = DT_DEBUG;
    dynamic.value = fake_dynamic + sizeof(lp64::Dynamic);
    CoreApi::Write(fake_dynamic, (void *)&dynamic, sizeof(lp64::Dynamic));

    lp64::Debug debug;
    debug.version = 1;
    debug.map = fake_link_map;
    CoreApi::Write(fake_dynamic + sizeof(lp64::Dynamic), (void *)&debug, sizeof(lp64::Debug));
}

void FakeCore::CreateFakeLinkMap(uint64_t fake_vma, uint64_t fake_link_map,
                                 std::set<std::string>& libs,
                                 std::vector<Opencore::VirtualMemoryArea>& maps) {
    LOGI("Create FAKE LINK MAP\n");
    int idx = 0;
    int num = libs.size();
    uint64_t entry_size = RoundUp(sizeof(lp64::LinkMap), 0x10);

    lp64::LinkMap fake;
    fake.addr = fake_vma,
    fake.name = 0x0;
    fake.ld = 0x0;
    fake.next = fake_link_map + (idx + 1) * entry_size;
    fake.prev = 0x0;
    CoreApi::Write(fake_link_map, (void *)&fake, sizeof(lp64::LinkMap));
    ++idx;

    for (const auto& lib : libs) {
        lp64::LinkMap link;
        link.addr = FindModuleLoad(maps, lib.c_str());
        link.name = 0x0;
        link.ld = 0x0;
        link.next = 0x0;
        link.prev = 0x0;
        if (idx < num) link.next = fake_link_map + (idx + 1) * entry_size;
        link.prev = fake_link_map + (idx - 1) * entry_size;
        CoreApi::Write(fake_link_map + idx * entry_size, (void *)&link, sizeof(lp64::LinkMap));
        ++idx;
    }
}

void FakeCore::CreateFakeStrtab(uint64_t fake_strtab, uint64_t fake_link_map, std::set<std::string>& libs) {
    LOGI("Create FAKE STRTAB\n");
    int idx = 0;
    uint64_t tmp_off = 0x0;
    uint64_t entry_size = RoundUp(sizeof(lp64::LinkMap), 0x10);

    std::string fake = FAKECORE_VMA;
    uint64_t length = fake.length() + 1;
    CoreApi::Write(fake_link_map + offsetof(lp64::LinkMap, name), fake_strtab + tmp_off);
    CoreApi::Write(fake_strtab + tmp_off, (void *)fake.data(), length);
    tmp_off += RoundUp(length, 0x10);
    ++idx;

    for (const auto& lib : libs) {
        std::size_t pos = lib.find(":");
        std::string libname;
        if (pos != std::string::npos)
            libname = lib.substr(0, pos);
        else
            libname = lib;
        length = libname.length() + 1;
        CoreApi::Write(fake_link_map + (idx * entry_size) + offsetof(lp64::LinkMap, name), fake_strtab + tmp_off);
        CoreApi::Write(fake_strtab + tmp_off, (void *)libname.data(), length);
        tmp_off += RoundUp(length, 0x10);
        ++idx;
    }
}

void FakeCore::Prepare(const char* filename) {
    LOGI("Create Fakecore %s ...\n", filename);
    memset(&ehdr, 0, sizeof(Elf64_Ehdr));
    memset(&note, 0, sizeof(Elf64_Phdr));
}

FakeCore::~FakeCore() {
    if (auxv) free(auxv);
    if (phdr) free(phdr);
    if (file) free(file);
}

} // namespace lp64
