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

#include "lp64/core.h"
#include "common/bit.h"
#include "common/exception.h"
#include <string.h>
#include <linux/elf.h>

namespace lp64 {

bool Core::load64(CoreApi* api, std::function<void* (uint64_t, uint64_t)> callback) {
    Elf64_Ehdr *ehdr = reinterpret_cast<Elf64_Ehdr *>(api->begin());
    Elf64_Phdr *phdr = reinterpret_cast<Elf64_Phdr *>(api->begin() + ehdr->e_phoff);

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
                Elf64_Nhdr *nhdr = reinterpret_cast<Elf64_Nhdr *>(pos);
                uint64_t item_pos = pos + sizeof(Elf64_Nhdr) + sizeof(uint64_t);
                switch(nhdr->n_type) {
                    case NT_PRSTATUS:
                        block->addThreadItem(callback(NT_PRSTATUS, item_pos));
                        break;
                    case NT_AUXV: {
                        int numauxv = nhdr->n_descsz / sizeof(lp64::Auxv);
                        lp64::Auxv* auxv = reinterpret_cast<lp64::Auxv *>(item_pos);
                        for (int index = 0; index < numauxv; ++index) {
                            block->addAuxvItem(auxv[index].type, auxv[index].value);
                        }
                    } break;
                    case NT_FILE: {
                        uint64_t numfile = reinterpret_cast<uint64_t *>(item_pos)[0];
                        uint64_t page_size = reinterpret_cast<uint64_t *>(item_pos)[1];
                        lp64::File *file = reinterpret_cast<lp64::File *>(item_pos
                                                        + sizeof(uint64_t) // NUMBER
                                                        + sizeof(uint64_t)); // PAGE_SIZE
                        uint64_t name_base = reinterpret_cast<uint64_t>(file) + sizeof(lp64::File) * numfile;
                        int index = 0;
                        while (index < numfile) {
                            block->addFileItem(file[index].begin, file[index].end, file[index].offset, name_base);
                            name_base += strlen(reinterpret_cast<const char *>(name_base)) + 1;
                            ++index;
                        }
                    } break;
                }

                pos = pos + RoundUp(nhdr->n_descsz, 0x4) + sizeof(Elf64_Nhdr) + sizeof(uint64_t);
            }

            api->addNoteBlock(block);
        }
    }
    return true;
}

uint64_t Core::loadDebug64(CoreApi* api) {
    uint64_t phdr = api->findAuxv(AT_PHDR);
    uint64_t phent = api->findAuxv(AT_PHENT);
    uint64_t phnum = api->findAuxv(AT_PHNUM);
    uint64_t execfn = api->findAuxv(AT_EXECFN);

    if (!CoreApi::IsVirtualValid(phdr)) {
        std::string name;
        if (CoreApi::IsVirtualValid(execfn)) {
            name.append(reinterpret_cast<const char*>(CoreApi::GetReal(execfn)));
        }
        std::cout << "Not found execfn [" << name << "]." << std::endl;
        return 0x0;
    }

    int index = 0;
    Elf64_Phdr *dyphdr = nullptr;
    Elf64_Phdr *cphdr = reinterpret_cast<Elf64_Phdr *>(CoreApi::GetReal(phdr));
    while (index < phnum) {
        if (cphdr[index].p_type == PT_DYNAMIC) {
            dyphdr = &cphdr[index];
            break;
        }
        index++;
    }

    if (dyphdr) {
        return FindDynamic(reinterpret_cast<uint64_t>(cphdr) - cphdr->p_vaddr, reinterpret_cast<uint64_t>(dyphdr), DT_DEBUG);
    }
    return 0x0;
}

void Core::loadLinkMap64(CoreApi* api) {
    uint64_t debug = CoreApi::GetDebug();
    if (!debug) {
        std::cout << "Not found debug." << std::endl;
        return;
    }

    try {
        Debug* dbg = reinterpret_cast<Debug *>(CoreApi::GetReal(debug));
        uint64_t map = dbg->map;
        while (map) {
            LinkMap* link = reinterpret_cast<LinkMap*>(CoreApi::GetReal(map));
            api->addLinkMap(link->addr, link->name);
            map = link->next;
        }
    } catch (InvalidAddressException e) {
        std::cout << e.what() << std::endl;
    }
}

uint64_t Core::FindDynamic(uint64_t load, uint64_t phdr, uint64_t type) {
    Elf64_Phdr *dyphdr = reinterpret_cast<Elf64_Phdr *>(phdr);
    Dynamic *dynamic = reinterpret_cast<Dynamic *>(load + dyphdr->p_vaddr);
    int numdynamic = dyphdr->p_filesz / sizeof(Dynamic);
    int index = 0;
    while (index < numdynamic) {
        if (dynamic[index].type == type) {
            return dynamic[index].value;
        }
        index++;
    }
    return 0x0;
}

} // namespace lp64
