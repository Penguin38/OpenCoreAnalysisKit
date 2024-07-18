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
#include "command/remote/opencore/arm64/opencore.h"
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <errno.h>

namespace arm64 {

struct user_pac_mask {
    uint64_t data_mask;
    uint64_t insn_mask;
};

void Opencore::CreateCorePrStatus(int pid) {
    if (!pids.size()) return;

    prnum = pids.size();
    prstatus = (Elf64_prstatus *)malloc(prnum * sizeof(Elf64_prstatus));
    memset(prstatus, 0, prnum * sizeof(Elf64_prstatus));

    for (int index = 0; index < prnum; index++) {
        pid_t tid = pids[index];
        prstatus[index].pr_pid = tid;

        uintptr_t regset = 1;
        struct iovec ioVec;

        ioVec.iov_base = &prstatus[index].pr_reg;
        ioVec.iov_len = sizeof(arm64::pt_regs);

        if (ptrace(PTRACE_GETREGSET, tid, regset, &ioVec) < 0) {
            LOGI("%s %d: %s\n", __func__ , tid, strerror(errno));
            continue;
        }
    }

    extra_note_filesz += (sizeof(Elf64_prstatus) + sizeof(Elf64_Nhdr) + 8) * prnum;
    extra_note_filesz += ((sizeof(struct user_pac_mask) + sizeof(Elf64_Nhdr) + 8)  // NT_ARM_PAC_MASK
                      + (sizeof(uint64_t) + sizeof(Elf64_Nhdr) + 8)       // NT_ARM_PAC_ENABLED_KEYS
                      ) * prnum;
    extra_note_filesz += (sizeof(uint64_t) + sizeof(Elf64_Nhdr) + 8) * prnum;  // NT_ARM_TAGGED_ADDR_CTRL
}

void Opencore::WriteCorePrStatus(FILE* fp) {
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(Elf64_prstatus);
    elf_nhdr.n_type = NT_PRSTATUS;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    int index = 0;
    while (index < prnum) {
        fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
        fwrite(magic, sizeof(magic), 1, fp);
        fwrite(&prstatus[index], sizeof(Elf64_prstatus), 1, fp);
        WriteCorePAC(prstatus[index].pr_pid ,fp);
        WriteCoreMTE(prstatus[index].pr_pid ,fp);
        index++;
    }
}

void Opencore::WriteCorePAC(int tid, FILE* fp) {
    // NT_ARM_PAC_MASK
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_LINUX_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(user_pac_mask);
    elf_nhdr.n_type = NT_ARM_PAC_MASK;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_LINUX_NAME_SZ, ELFLINUXMAGIC);

    fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    user_pac_mask uregs;
    struct iovec pac_mask_iov = {
        &uregs,
        sizeof(user_pac_mask),
    };
    if (ptrace(PTRACE_GETREGSET, tid, NT_ARM_PAC_MASK,
                reinterpret_cast<void*>(&pac_mask_iov)) == -1) {
        uint64_t mask = GENMASK(54, DEF_VA_BITS);
        uregs.data_mask = mask;
        uregs.insn_mask = mask;
    }
    fwrite(&uregs, sizeof(user_pac_mask), 1, fp);

    // NT_ARM_PAC_ENABLED_KEYS
    elf_nhdr.n_descsz = sizeof(uint64_t);
    elf_nhdr.n_type = NT_ARM_PAC_ENABLED_KEYS;

    fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    uint64_t pac_enabled_keys;
    struct iovec pac_enabled_keys_iov = {
        &pac_enabled_keys,
        sizeof(pac_enabled_keys),
    };
    if (ptrace(PTRACE_GETREGSET, tid, NT_ARM_PAC_ENABLED_KEYS,
                reinterpret_cast<void*>(&pac_enabled_keys_iov)) == -1) {
        pac_enabled_keys = -1;
    }
    fwrite(&pac_enabled_keys, sizeof(uint64_t), 1, fp);
}

void Opencore::WriteCoreMTE(int tid, FILE* fp) {
    // NT_ARM_TAGGED_ADDR_CTRL
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_LINUX_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(uint64_t);
    elf_nhdr.n_type = NT_ARM_TAGGED_ADDR_CTRL;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_LINUX_NAME_SZ, ELFLINUXMAGIC);

    fwrite(&elf_nhdr, sizeof(Elf64_Nhdr), 1, fp);
    fwrite(magic, sizeof(magic), 1, fp);

    uint64_t tagged_addr_ctrl;
    struct iovec tagged_addr_ctrl_iov = {
        &tagged_addr_ctrl,
        sizeof(tagged_addr_ctrl),
    };
    if (ptrace(PTRACE_GETREGSET, tid, NT_ARM_TAGGED_ADDR_CTRL,
                reinterpret_cast<void*>(&tagged_addr_ctrl_iov)) == -1) {
        tagged_addr_ctrl = -1;
    }
    fwrite(&tagged_addr_ctrl, sizeof(uint64_t), 1, fp);
}

Opencore::~Opencore() {
    if (prstatus) free(prstatus);
}

} // namespace arm64
