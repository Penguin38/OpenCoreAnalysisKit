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
#include "common/bit.h"
#include "common/elf.h"
#include "command/fake/core/lp64/restore.h"
#include <stdio.h>
#include <linux/elf.h>

namespace lp64 {

int Restore::execute(const char* output) {
    if (!CoreApi::IsReady())
        return -1;

    FILE *fp = fopen(output, "wb");
    if (!fp) {
        LOGE("Can't open \"%s\".\n", output);
        return -1;
    }

    Elf64_Ehdr *ehdr = reinterpret_cast<Elf64_Ehdr *>(CoreApi::GetBegin());
    Elf64_Phdr *phdr = reinterpret_cast<Elf64_Phdr *>(CoreApi::GetBegin() + ehdr->e_phoff);

    // Write ELF Header
    fwrite(ehdr, sizeof(Elf64_Ehdr), 1, fp);

    uint64_t current_offset = 0x0;
    Elf64_Phdr *tmp = reinterpret_cast<Elf64_Phdr *>(malloc(sizeof(Elf64_Phdr) * ehdr->e_phnum));
    // Write Program Headers
    for (int num = 0; num < ehdr->e_phnum; ++num) {
        tmp[num].p_type = phdr[num].p_type;
        tmp[num].p_flags = phdr[num].p_flags;
        tmp[num].p_offset = phdr[num].p_offset + current_offset;
        tmp[num].p_vaddr = phdr[num].p_vaddr;
        tmp[num].p_paddr = phdr[num].p_paddr;
        tmp[num].p_filesz = phdr[num].p_filesz;
        tmp[num].p_memsz = phdr[num].p_memsz;
        tmp[num].p_align = phdr[num].p_align;

        if (phdr[num].p_type == PT_LOAD && !phdr[num].p_filesz) {
            LoadBlock* block = CoreApi::FindLoadBlock(phdr[num].p_vaddr, false);
            if (block && block->isValid()) {
                tmp[num].p_filesz = tmp[num].p_memsz;
                current_offset += tmp[num].p_filesz;
            }
        }

        fwrite(&tmp[num], sizeof(Elf64_Phdr), 1, fp);
    }

    uint64_t current_filesz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * ehdr->e_phnum;
    // Write Segment
    uint8_t* zero_buf = (uint8_t*)malloc(ELF_PAGE_SIZE);
    memset(zero_buf, 0x0, ELF_PAGE_SIZE);
    for (int num = 0; num < ehdr->e_phnum; ++num) {
        if (!tmp[num].p_filesz)
            continue;

        if (phdr[num].p_type == PT_LOAD) {
            LoadBlock* block = CoreApi::FindLoadBlock(phdr[num].p_vaddr, false);
            if (block && block->isValid()) {
                current_filesz += tmp[num].p_filesz;
                fwrite(reinterpret_cast<void *>(block->begin()), tmp[num].p_filesz, 1, fp);
                continue;
            }
        }

        fwrite(reinterpret_cast<void *>(CoreApi::GetBegin() + phdr[num].p_offset), tmp[num].p_filesz, 1, fp);
        current_filesz += tmp[num].p_filesz;
        if (!IS_ALIGNED(current_filesz, ELF_PAGE_SIZE)) {
            uint64_t aliged_size = RoundUp(current_filesz, ELF_PAGE_SIZE) - current_filesz;
            fwrite(zero_buf, aliged_size, 1, fp);
            current_filesz += aliged_size;
        }
    }

    free(zero_buf);
    free(tmp);
    fclose(fp);
    LOGI("FakeCore: saved [%s]\n", output);
    return 0;
}

} // namespace lp64
