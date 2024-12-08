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
#include "command/fake/core/lp64/fake_core.h"
#include <stdio.h>
#include <string.h>
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
        tmp[num].p_memsz = note->size();
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
        tmp[num].p_memsz = block->size();
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

} // namespace lp64
