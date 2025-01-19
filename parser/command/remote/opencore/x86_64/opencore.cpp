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
#include "command/remote/opencore/x86_64/opencore.h"
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <errno.h>

namespace x86_64 {

void Opencore::CreateCorePrStatus(int pid) {
    if (!pids.size()) return;

    prnum = pids.size();
    prstatus = (Elf64_prstatus *)malloc(prnum * sizeof(Elf64_prstatus));
    memset(prstatus, 0, prnum * sizeof(Elf64_prstatus));

    int cur = 1;
    for (int index = 0; index < prnum; index++) {
        pid_t tid = pids[index];
        int idx;
        if (tid == getTid()) {
            idx = 0;
            prstatus[idx].pr_pid = tid;
        } else {
            // 0 top thread was truncated
            idx = (cur >= prnum) ? 0 : cur;
            ++cur;
            prstatus[idx].pr_pid = tid;
        }

        struct iovec ioVec = {
            &prstatus[idx].pr_reg,
            sizeof(x86_64::pt_regs),
        };

        if (ptrace(PTRACE_GETREGSET, tid, NT_PRSTATUS, &ioVec) < 0) {
            LOGD("%s %d: %s\n", __func__ , tid, strerror(errno));
            continue;
        }
    }

    extra_note_filesz += (sizeof(Elf64_prstatus) + sizeof(Elf64_Nhdr) + 8) * prnum;
}

void Opencore::WriteCorePrStatus(FILE* fp) {
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(Elf64_prstatus);
    elf_nhdr.n_type = NT_PRSTATUS;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    for (int index = 0; index < prnum; index++) {
        fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
        fwrite(magic, sizeof(magic), 1, fp);
        fwrite(&prstatus[index], sizeof(Elf64_prstatus), 1, fp);
    }
}

bool Opencore::IsSpecialFilterSegment(Opencore::VirtualMemoryArea& vma, int idx) {
    int filter = getFilter();
    if (filter & FILTER_MINIDUMP) {
        if (!prnum)
            return true;

        x86_64::pt_regs *regs = &prstatus[0].pr_reg;
        if (regs->r15 >= vma.begin && regs->r15 < vma.end
                || regs->r14 >= vma.begin && regs->r14 < vma.end
                || regs->r13 >= vma.begin && regs->r13 < vma.end
                || regs->r12 >= vma.begin && regs->r12 < vma.end
                || regs->rbp >= vma.begin && regs->rbp < vma.end
                || regs->rbx >= vma.begin && regs->rbx < vma.end
                || regs->r11 >= vma.begin && regs->r11 < vma.end
                || regs->r10 >= vma.begin && regs->r10 < vma.end
                || regs->r9 >= vma.begin && regs->r9 < vma.end
                || regs->r8 >= vma.begin && regs->r8 < vma.end
                || regs->rax >= vma.begin && regs->rax < vma.end
                || regs->rcx >= vma.begin && regs->rcx < vma.end
                || regs->rdx >= vma.begin && regs->rdx < vma.end
                || regs->rsi >= vma.begin && regs->rsi < vma.end
                || regs->rdi >= vma.begin && regs->rdi < vma.end
                || regs->rip >= vma.begin && regs->rip < vma.end) {
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

} // namespace x86_64
