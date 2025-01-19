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
#include "command/remote/opencore/x86/opencore.h"
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <errno.h>

namespace x86 {

void Opencore::CreateCorePrStatus(int pid) {
    if (!pids.size()) return;

    prnum = pids.size();
    prstatus = (Elf32_prstatus *)malloc(prnum * sizeof(Elf32_prstatus));
    memset(prstatus, 0, prnum * sizeof(Elf32_prstatus));

    for (int index = 0; index < prnum; index++) {
        pid_t tid = pids[index];
        prstatus[index].pr_pid = tid;

        struct iovec ioVec = {
            &prstatus[index].pr_reg,
            sizeof(x86::pt_regs),
        };

        if (ptrace(PTRACE_GETREGSET, tid, NT_PRSTATUS, &ioVec) < 0) {
            LOGD("%s %d: %s\n", __func__ , tid, strerror(errno));
            continue;
        }
    }

    extra_note_filesz += (sizeof(Elf32_prstatus) + sizeof(Elf32_Nhdr) + 8) * prnum;
}

void Opencore::WriteCorePrStatus(FILE* fp) {
    Elf32_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(Elf32_prstatus);
    elf_nhdr.n_type = NT_PRSTATUS;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    int index = 0;
    while (index < prnum) {
        fwrite(&elf_nhdr, sizeof(Elf32_Nhdr), 1, fp);
        fwrite(magic, sizeof(magic), 1, fp);
        fwrite(&prstatus[index], sizeof(Elf32_prstatus), 1, fp);
        index++;
    }
}

bool Opencore::IsSpecialFilterSegment(Opencore::VirtualMemoryArea& vma, int idx) {
    int filter = getFilter();
    if (filter & FILTER_MINIDUMP) {
        if (!prnum)
            return true;

        x86::pt_regs *regs = &prstatus[0].pr_reg;
        if (regs->ebx >= vma.begin && regs->ebx < vma.end
                || regs->edx >= vma.begin && regs->edx < vma.end
                || regs->esi >= vma.begin && regs->esi < vma.end
                || regs->edi >= vma.begin && regs->edi < vma.end
                || regs->ebp >= vma.begin && regs->ebp < vma.end
                || regs->eax >= vma.begin && regs->eax < vma.end
                || regs->eip >= vma.begin && regs->eip < vma.end
                || regs->esp >= vma.begin && regs->esp < vma.end) {
            phdr[idx].p_filesz = phdr[idx].p_memsz;
            return false;
        }

        return true;
    }
    return false;
}

Opencore::~Opencore() {
    if (prstatus) free(prstatus);
}

} // namespace x86
