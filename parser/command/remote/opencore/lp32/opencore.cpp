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
#include "common/bit.h"
#include "common/elf.h"
#include "command/remote/opencore/lp32/opencore.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace lp32 {

void OpencoreImpl::ParserPhdr(int index, uint32_t start, uint32_t end, char* flags, char* filename) {
    phdr[index].p_type = PT_LOAD;

    phdr[index].p_vaddr = (Elf32_Addr)start;
    phdr[index].p_paddr = 0x0;
    phdr[index].p_memsz = (Elf32_Addr)end-(Elf32_Addr)start;

    if (flags[0] == 'r' || flags[0] == 'R')
        phdr[index].p_flags = phdr[index].p_flags | PF_R;

    if (flags[1] == 'w' || flags[1] == 'W')
        phdr[index].p_flags = phdr[index].p_flags | PF_W;

    if (flags[2] == 'x' || flags[2] == 'X')
        phdr[index].p_flags = phdr[index].p_flags | PF_X;

    phdr[index].p_filesz = phdr[index].p_memsz;
    phdr[index].p_align = align_size;
}

void OpencoreImpl::ParserNtFile(int index, uint32_t start, uint32_t end, int fileofs, char* filename) {
    file[index].begin = (Elf32_Addr)start;
    file[index].end = (Elf32_Addr)end;
    file[index].offset = fileofs >> 12;

    int len = strlen(filename);
    buffer.insert(buffer.end(), filename, filename + len);
    char empty[1] = {'\0'};
    buffer.insert(buffer.end(), empty, empty + 1);
    fileslen += len + 1;
    maps[file[index].begin] = filename;
}

void OpencoreImpl::ParseProcessMapsVma(int pid) {
    char filename[32];
    char line[1024];

    snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    FILE *fp = fopen(filename, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            phnum++;
        }
        fseek(fp, 0, SEEK_SET);

        phdr = (Elf32_Phdr *)malloc(phnum * sizeof(Elf32_Phdr));
        memset(phdr, 0, phnum * sizeof(Elf32_Phdr));
        file = (lp32::File *)malloc(phnum * sizeof(lp32::File));
        memset(file, 0, phnum * sizeof(lp32::File));

        int index = 0;
        while (fgets(line, sizeof(line), fp)) {
            int m, fileofs, inode;
            uint32_t start, end;
            char flags[4];
            char filename[256];

            sscanf(line, "%x-%x %c%c%c%c %x %*x:%*x  %u %[^\n] %n",
                   &start, &end,
                   &flags[0], &flags[1], &flags[2], &flags[3],
                   &fileofs, &inode, filename, &m);

            ParserPhdr(index, start, end, flags, filename);
            ParserNtFile(index, start, end, fileofs, filename);

            if (IsFilterSegment(flags, inode, filename, fileofs)) {
                phdr[index].p_filesz = 0x0;
            }

            index++;
        }
        fclose(fp);
    }
}

void OpencoreImpl::CreateCoreHeader() {
    snprintf((char *)ehdr.e_ident, 5, ELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS32;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_CORE;
    ehdr.e_machine = getMachine();
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0x0;
    ehdr.e_phoff = sizeof(Elf32_Ehdr);
    ehdr.e_shoff = 0x0;
    ehdr.e_flags = 0x0;
    ehdr.e_ehsize = sizeof(Elf32_Ehdr);
    ehdr.e_phentsize = sizeof(Elf32_Phdr);
    ehdr.e_phnum = phnum + 1;
    ehdr.e_shentsize = 0x0;
    ehdr.e_shnum = 0x0;
    ehdr.e_shstrndx = 0x0;
}

void OpencoreImpl::CreateCoreNoteHeader() {
    note.p_type = PT_NOTE;
    note.p_offset = sizeof(Elf32_Ehdr) + ehdr.e_phnum * sizeof(Elf32_Phdr);
}

void OpencoreImpl::CreateCoreAUXV(int pid) {
    char filename[32];
    lp32::Auxv vec;
    snprintf(filename, sizeof(filename), "/proc/%d/auxv", pid);

    FILE *fp = fopen(filename, "rb");
    if (fp != nullptr) {
        while (fread(&vec, sizeof(vec), 1, fp)) {
            auxvnum++;
        }
        fseek(fp, 0, SEEK_SET);

        auxv = (lp32::Auxv *)malloc(auxvnum * sizeof(lp32::Auxv));
        memset(auxv, 0, auxvnum * sizeof(lp32::Auxv));

        int index =0;
        while (fread(&vec, sizeof(vec), 1, fp)) {
            auxv[index].type = vec.type;
            auxv[index].value = vec.value;
            index++;
        }

        fclose(fp);
    }
}

void OpencoreImpl::WriteCoreHeader(FILE* fp) {
    fwrite((void *)&ehdr, sizeof(Elf32_Ehdr), 1, fp);
}

void OpencoreImpl::WriteCoreNoteHeader(FILE* fp) {
    note.p_filesz += sizeof(lp32::Auxv) * auxvnum + sizeof(Elf32_Nhdr) + 8;
    note.p_filesz += extra_note_filesz;
    note.p_filesz += sizeof(lp32::File) * phnum + sizeof(Elf32_Nhdr) + 8 + 2 * 4 + fileslen;
    fwrite((void *)&note, sizeof(Elf32_Phdr), 1, fp);
}

void OpencoreImpl::WriteCoreProgramHeaders(FILE* fp) {
    uint32_t offset = RoundUp(note.p_offset + note.p_filesz, align_size);
    phdr[0].p_offset = offset;
    fwrite(&phdr[0], sizeof(Elf32_Phdr), 1, fp);

    int index = 1;
    while (index < ehdr.e_phnum - 1) {
        phdr[index].p_offset = phdr[index - 1].p_offset + phdr[index-1].p_filesz;
        fwrite(&phdr[index], sizeof(Elf32_Phdr), 1, fp);
        index++;
    }
}

void OpencoreImpl::WriteCoreAUXV(FILE* fp) {
    Elf32_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(lp32::Auxv) * auxvnum;
    elf_nhdr.n_type = NT_AUXV;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    fwrite(&elf_nhdr, sizeof(Elf32_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    int index = 0;
    while (index < auxvnum) {
        fwrite(&auxv[index], sizeof(lp32::Auxv), 1, fp);
        index++;
    }
}

void OpencoreImpl::WriteNtFile(FILE* fp) {
    Elf32_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(lp32::File) * phnum + 2 * 4 + fileslen;
    elf_nhdr.n_type = NT_FILE;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    fwrite(&elf_nhdr, sizeof(Elf32_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    uint32_t number = phnum;
    fwrite(&number, 4, 1, fp);
    fwrite(&page_size, 4, 1, fp);

    int index = 0;
    while(index < phnum){
        fwrite(&file[index], sizeof(lp32::File), 1, fp);
        index++;
    }

    fwrite(buffer.data(), buffer.size(), 1, fp);
}

void OpencoreImpl::AlignNoteSegment(FILE* fp) {
    memset(zero, 0x0, align_size);
    uint32_t offset = RoundUp(note.p_offset + note.p_filesz, align_size);
    uint32_t size = offset - (note.p_offset + note.p_filesz);
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

    while(index < ehdr.e_phnum - 1) {
        if (phdr[index].p_filesz > 0) {
            long current_pos = ftell(fp);
            bool need_padd_zero = false;
            int count = phdr[index].p_memsz / align_size;
            for (int i = 0; i < count; i++) {
                memset(zero, 0x0, align_size);
                pread64(fd, zero, phdr[index].p_align, phdr[index].p_vaddr + (i * align_size));
                uint32_t ret = fwrite(zero, align_size, 1, fp);
                if (ret != 1) {
                    need_padd_zero = true;
                    LOGE("[%x] write load segment fail. %s %s\n",
                            (uint32_t)phdr[index].p_vaddr, strerror(errno), maps[file[index].begin].c_str());
                    break;
                }
            }
            if (need_padd_zero && current_pos > 0) {
                memset(zero, 0x0, align_size);
                fseek(fp, current_pos, SEEK_SET);
                int count = phdr[index].p_memsz / align_size;
                for (int i = 0; i < count; i++) {
                    uint32_t ret = fwrite(zero, align_size, 1, fp);
                    if (ret != 1) {
                        LOGE("[%x] padding load segment fail. %s %s\n",
                                (uint32_t)phdr[index].p_vaddr, strerror(errno), maps[file[index].begin].c_str());
                    }
                }
            }
        }
        index++;
    }
    close(fd);
}

bool OpencoreImpl::DoCoredump(const char* filename) {
    Prepare(filename);

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

bool OpencoreImpl::NeedFilterFile(const char* filename, int offset) {
    struct stat sb;
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
        return true;

    if (fstat(fd, &sb) < 0) {
        close(fd);
        return true;
    }

    char* mem = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0x0);
    close(fd);
    if (mem == MAP_FAILED)
        return true;

    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)mem;
    if (strncmp(mem, ELFMAG, 4) || ehdr->e_machine != getMachine()) {
        munmap(mem, sb.st_size);
        return true;
    }

    bool ret = true;
    Elf32_Phdr* phdr = (Elf32_Phdr *)(mem + ehdr->e_phoff);
    for (int index = 0; index < ehdr->e_phnum; index++) {
        if (phdr[index].p_type != PT_LOAD)
            continue;

        int pos = RoundDown(phdr[index].p_offset, page_size);
        int end = pos + phdr[index].p_memsz;
        if (pos <= offset && offset < end) {
            if ((phdr[index].p_flags & PF_W))
                ret = false;
        }
    }
    return ret;
}

void OpencoreImpl::Prepare(const char* filename) {
    LOGI("Coredump %s ...\n", filename);
    memset(&ehdr, 0, sizeof(Elf32_Ehdr));
    memset(&note, 0, sizeof(Elf32_Phdr));
    pids.clear();
    buffer.clear();
    maps.clear();
}

OpencoreImpl::~OpencoreImpl() {
    if (auxv) free(auxv);
    if (phdr) free(phdr);
    if (file) free(file);
    LOGI("Finish done.\n");
}

} // namespace lp32
