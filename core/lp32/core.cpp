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

#include "lp32/core.h"
#include "common/bit.h"
#include <string.h>
#include <linux/elf.h>

namespace lp32 {

bool Core::load32(CoreApi* api, std::function<void* (uint64_t, uint64_t)> callback) {
    Elf32_Ehdr *ehdr = reinterpret_cast<Elf32_Ehdr *>(api->begin());
    Elf32_Phdr *phdr = reinterpret_cast<Elf32_Phdr *>(api->begin() + ehdr->e_phoff);

    for (int num = 0; num < ehdr->e_phnum; ++num) {
        if (phdr[num].p_type == PT_LOAD) {
            std::shared_ptr<LoadBlock> block(new LoadBlock(phdr[num].p_flags,
                                             phdr[num].p_offset,
                                             phdr[num].p_vaddr,
                                             phdr[num].p_paddr,
                                             phdr[num].p_filesz,
                                             phdr[num].p_memsz,
                                             phdr[num].p_align));
            block->setOriAddr(api->begin() + phdr[num].p_offset);
            api->addLoadBlock(block);
        } else if (phdr[num].p_type == PT_NOTE) {
            std::unique_ptr<NoteBlock> block(new NoteBlock(phdr[num].p_flags,
                                             phdr[num].p_offset,
                                             phdr[num].p_vaddr,
                                             phdr[num].p_paddr,
                                             phdr[num].p_filesz,
                                             phdr[num].p_memsz,
                                             phdr[num].p_align));
            block->setOriAddr(api->begin() + phdr[num].p_offset);

            if (!block->isValidBlock())
                return false;

            uint64_t pos = block->oraddr();
            uint64_t end = block->oraddr() + block->realSize();
            while (pos < end) {
                Elf32_Nhdr *nhdr = reinterpret_cast<Elf32_Nhdr *>(pos);
                uint64_t item_pos = pos + sizeof(Elf32_Nhdr) + sizeof(uint64_t);
                switch(nhdr->n_type) {
                    case NT_PRSTATUS:
                        block->addThreadItem(callback(NT_PRSTATUS, item_pos));
                        break;
                    case NT_AUXV: {
                        int numauxv = nhdr->n_descsz / sizeof(lp32::Auxv);
                        lp32::Auxv* auxv = reinterpret_cast<lp32::Auxv *>(item_pos);
                        for (int index = 0; index < numauxv; ++index) {
                            block->addAuxvItem(auxv[index].type, auxv[index].value);
                        }
                    } break;
                    case NT_FILE: {
                        uint32_t numfile = reinterpret_cast<uint32_t *>(item_pos)[0];
                        uint32_t page_size = reinterpret_cast<uint32_t *>(item_pos)[1];
                        lp32::File *file = reinterpret_cast<lp32::File *>(item_pos
                                                        + sizeof(uint32_t) // NUMBER
                                                        + sizeof(uint32_t)); // PAGE_SIZE
                        uint64_t name_base = reinterpret_cast<uint64_t>(file) + sizeof(lp32::File) * numfile;
                        int index = 0;
                        while (index < numfile) {
                            block->addFileItem(file[index].begin, file[index].end, file[index].offset, name_base);
                            name_base += strlen(reinterpret_cast<const char *>(name_base)) + 1;
                            ++index;
                        }
                    } break;
                }

                pos = pos + RoundUp(nhdr->n_descsz, 0x4) + sizeof(Elf32_Nhdr) + sizeof(uint64_t);
            }

            api->addNoteBlock(block);
        }
    }
    return true;
}

} // namespace lp32
