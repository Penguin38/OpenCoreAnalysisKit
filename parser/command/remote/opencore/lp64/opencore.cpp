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
#include "common/bit.h"
#include "common/elf.h"
#include "base/memory_map.h"
#include "command/remote/opencore/lp64/opencore.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>

namespace lp64 {

void OpencoreImpl::ParserPhdr(int index, Opencore::VirtualMemoryArea& vma) {
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

    phdr[index].p_filesz = phdr[index].p_memsz;
    phdr[index].p_align = align_size;
}

void OpencoreImpl::ParserNtFile(int index, Opencore::VirtualMemoryArea& vma) {
    file[index].begin = (Elf64_Addr)vma.begin;
    file[index].end = (Elf64_Addr)vma.end;
    file[index].offset = vma.offset >> 12;
    fileslen += vma.file.length() + 1;
}

void OpencoreImpl::ParseProcessMapsVma(int pid) {
    ParseMaps(pid, maps);
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

void OpencoreImpl::CreateCoreHeader() {
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

void OpencoreImpl::CreateCoreNoteHeader() {
    note.p_type = PT_NOTE;
    note.p_offset = sizeof(Elf64_Ehdr) + ehdr.e_phnum * sizeof(Elf64_Phdr);
}

void OpencoreImpl::CreateCoreAUXV(int pid) {
    char filename[32];
    lp64::Auxv vec;
    snprintf(filename, sizeof(filename), "/proc/%d/auxv", pid);

    FILE *fp = fopen(filename, "rb");
    if (fp != nullptr) {
        while (fread(&vec, sizeof(vec), 1, fp)) {
            auxvnum++;
        }
        fseek(fp, 0, SEEK_SET);

        auxv = (lp64::Auxv *)malloc(auxvnum * sizeof(lp64::Auxv));
        memset(auxv, 0, auxvnum * sizeof(lp64::Auxv));

        int index =0;
        while (fread(&vec, sizeof(vec), 1, fp)) {
            auxv[index].type = vec.type;
            auxv[index].value = vec.value;
            index++;
        }

        fclose(fp);
    }
}

void OpencoreImpl::SpecialCoreFilter() {
    for (int index = 0; index < maps.size(); ++index) {
        Opencore::VirtualMemoryArea& vma = maps[index];
        if (IsFilterSegment(vma))
            phdr[index].p_filesz = 0x0;

        if (IsSpecialFilterSegment(vma, index))
            phdr[index].p_filesz = 0x0;
    }
}

void OpencoreImpl::WriteCoreHeader(FILE* fp) {
    fwrite((void *)&ehdr, sizeof(Elf64_Ehdr), 1, fp);
}

void OpencoreImpl::WriteCoreNoteHeader(FILE* fp) {
    note.p_filesz += sizeof(lp64::Auxv) * auxvnum + sizeof(Elf64_Nhdr) + 8;
    note.p_filesz += extra_note_filesz;
    note.p_filesz += sizeof(lp64::File) * phnum + sizeof(Elf64_Nhdr) + 8 + 2 * 8 + RoundUp(fileslen, 4);
    fwrite((void *)&note, sizeof(Elf64_Phdr), 1, fp);
}

void OpencoreImpl::WriteCoreProgramHeaders(FILE* fp) {
    if (!phnum)
        return;

    uint64_t offset = RoundUp(note.p_offset + note.p_filesz, align_size);
    phdr[0].p_offset = offset;
    fwrite(&phdr[0], sizeof(Elf64_Phdr), 1, fp);

    int index = 1;
    while (index < phnum) {
        phdr[index].p_offset = phdr[index - 1].p_offset + phdr[index-1].p_filesz;
        fwrite(&phdr[index], sizeof(Elf64_Phdr), 1, fp);
        index++;
    }
}

void OpencoreImpl::WriteCoreAUXV(FILE* fp) {
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(lp64::Auxv) * auxvnum;
    elf_nhdr.n_type = NT_AUXV;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    int index = 0;
    while (index < auxvnum) {
        fwrite(&auxv[index], sizeof(lp64::Auxv), 1, fp);
        index++;
    }
}

void OpencoreImpl::WriteNtFile(FILE* fp) {
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(lp64::File) * phnum + 2 * 8 + RoundUp(fileslen, 4);
    elf_nhdr.n_type = NT_FILE;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    uint64_t number = phnum;
    fwrite(&number, 8, 1, fp);
    fwrite(&page_size, 8, 1, fp);

    for (int index = 0; index < phnum; ++index)
        fwrite(&file[index], sizeof(lp64::File), 1, fp);

    for (int index = 0; index < phnum; ++index)
        fwrite(maps[index].file.data(), maps[index].file.length() + 1, 1, fp);
}

void OpencoreImpl::AlignNoteSegment(FILE* fp) {
    memset(zero, 0x0, align_size);
    uint64_t align_filesz = note.p_filesz - RoundUp(fileslen, 4) + fileslen;
    uint64_t offset = RoundUp(note.p_offset + align_filesz, align_size);
    uint64_t size = offset - (note.p_offset + align_filesz);
    fwrite(zero, size, 1, fp);
}

void OpencoreImpl::WriteCoreLoadSegment(int pid, FILE* fp) {
    char filename[32];
    int fd;
    int index = 0;
    memset(zero, 0x0, align_size);

    snprintf(filename, sizeof(filename), "/proc/%d/mem", pid);
    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        LOGE("open %s fail.\n", filename);
        return;
    }

    while(index < phnum) {
        if (phdr[index].p_filesz > 0) {
            long current_pos = ftell(fp);
            bool need_padd_zero = false;
            int count = phdr[index].p_memsz / align_size;
            for (int i = 0; i < count; i++) {
                memset(zero, 0x0, align_size);
                pread64(fd, zero, phdr[index].p_align, phdr[index].p_vaddr + (i * align_size));
                uint64_t ret = fwrite(zero, align_size, 1, fp);
                if (ret != 1) {
                    LOGE("[%" PRIx64 "] write load segment fail. %s %s\n",
                            (uint64_t)phdr[index].p_vaddr, strerror(errno), maps[index].file.c_str());
                    if (errno == ENOSPC)
                        return;

                    need_padd_zero = true;
                    break;
                }
            }
            if (need_padd_zero && current_pos > 0) {
                memset(zero, 0x0, align_size);
                fseek(fp, current_pos, SEEK_SET);
                int count = phdr[index].p_memsz / align_size;
                for (int i = 0; i < count; i++)
                    fwrite(zero, align_size, 1, fp);
            }
        }
        index++;
    }
    close(fd);
}

bool OpencoreImpl::DoCoredump(const char* filename) {
    Prepare(filename);

    if (!Opencore::isAlive(getPid()))
        return false;

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        LOGE("%s %s: %s\n", __func__, filename, strerror(errno));
        return false;
    }

    StopTheWorld(getPid());

    ParseProcessMapsVma(getPid());
    CreateCoreHeader();
    CreateCoreNoteHeader();
    CreateCorePrStatus(getPid());
    CreateCoreAUXV(getPid());
    SpecialCoreFilter();

    // ELF Header
    WriteCoreHeader(fp);

    // Program Headers
    WriteCoreNoteHeader(fp);
    WriteCoreProgramHeaders(fp);

    // Segments
    WriteCorePrStatus(fp);
    WriteCoreAUXV(fp);
    WriteNtFile(fp);
    AlignNoteSegment(fp);
    WriteCoreLoadSegment(getPid(), fp);

    fclose(fp);
    return true;
}

bool OpencoreImpl::NeedFilterFile(Opencore::VirtualMemoryArea& vma) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(vma.file.c_str()));
    if (!map) return true;

    char* mem = (char*)map->data();
    Elf64_Ehdr* ehdr = (Elf64_Ehdr*)mem;
    if (strncmp(mem, ELFMAG, 4) || ehdr->e_machine != getMachine())
        return true;

    bool ret = true;
    Elf64_Phdr* phdr = (Elf64_Phdr *)(mem + ehdr->e_phoff);
    for (int index = 0; index < ehdr->e_phnum; index++) {
        if (phdr[index].p_type != PT_LOAD)
            continue;

        int pos = RoundDown(phdr[index].p_offset, page_size);
        int end = pos + phdr[index].p_memsz;
        if (pos <= vma.offset && vma.offset < end) {
            if ((phdr[index].p_flags & PF_W))
                ret = false;
        }
    }
    return ret;
}

uint64_t OpencoreImpl::FindAuxv(uint64_t type) {
    for (int idx = 0; idx < auxvnum; ++idx) {
        if (auxv[idx].type == type)
            return auxv[idx].value;
    }
    return 0;
}

void OpencoreImpl::Prepare(const char* filename) {
    LOGI("Coredump %s ...\n", filename);
    zero = (uint8_t*)malloc(align_size);
    memset(&ehdr, 0, sizeof(Elf64_Ehdr));
    memset(&note, 0, sizeof(Elf64_Phdr));
}

OpencoreImpl::~OpencoreImpl() {
    if (auxv) free(auxv);
    if (phdr) free(phdr);
    if (file) free(file);
    if (zero) free(zero);
    LOGI("Finish done.\n");
}

} // namespace lp64
