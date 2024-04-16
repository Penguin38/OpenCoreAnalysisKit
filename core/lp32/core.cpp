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
#include "lp32/core.h"
#include "api/memory_ref.h"
#include "common/elf.h"
#include "common/bit.h"
#include "common/load_block.h"
#include "common/exception.h"
#include <string.h>
#include <linux/elf.h>

bool lp32::Core::load32(CoreApi* api, std::function<void* (uint64_t, uint64_t)> callback) {
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
            block->setTruncated(api->size() < (phdr[num].p_offset + phdr[num].p_filesz));
            if (!(block->flags() & Block::FLAG_R))
                continue;

            block->setOriAddr(api->begin() + phdr[num].p_offset);
            block->setVabitsMask(CoreApi::GetVabitsMask());
            block->setPointMask(CoreApi::GetPointMask());
            api->addLoadBlock(block);
        } else if (phdr[num].p_type == PT_NOTE) {
            std::unique_ptr<NoteBlock> block(new NoteBlock(phdr[num].p_flags,
                                             phdr[num].p_offset,
                                             phdr[num].p_vaddr,
                                             phdr[num].p_paddr,
                                             phdr[num].p_filesz,
                                             phdr[num].p_memsz,
                                             phdr[num].p_align));
            block->setTruncated(api->size() < (phdr[num].p_offset + phdr[num].p_filesz));
            if (!block->isValidBlock())
                continue;

            block->setOriAddr(api->begin() + phdr[num].p_offset);
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
    LOGI("Core load (%p) %s\n", this, api->getName().c_str());
    return true;
}

void lp32::Core::loadLinkMap32(CoreApi* api) {
    api::MemoryRef& debug = GetDebug();
    if (!debug.Ptr()) {
        LOGW("WARN: Not found debug. You can try command exec.\n");
        return;
    }

    try {
        lp32::Debug* dbg = reinterpret_cast<lp32::Debug*>(debug.Real());
        uint32_t map = dbg->map;
        while (map) {
            lp32::LinkMap* link = reinterpret_cast<lp32::LinkMap*>(CoreApi::GetReal(map));
            api->addLinkMap(map, link->addr, link->name);
            map = link->next;
        }
    } catch (InvalidAddressException e) {
        LOGI("%s\n", e.what());
    }
}

bool lp32::Core::dlopen32(CoreApi* api, uint32_t begin, const char* file) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(file));
    if (map) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
        if (memcmp(header->ident, ELFMAG, 4)) {
            LOGE("ERROR: Invalid ELF file (%s)\n", file);
            return false;
        }

        if (header->type != ET_DYN) {
            LOGE("ERROR: Invalid shared object file (%s)\n", file);
            return false;
        }

        if (header->machine != CoreApi::GetMachine()) {
            LOGE("ERROR: Invalid match machine(%d) (%s)\n", header->machine, file);
            return false;
        }

        Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
        Elf32_Phdr* phdr = reinterpret_cast<Elf32_Phdr*>(map->data() + ehdr->e_phoff);

        LoadBlock* begin_block = api->findLoadBlock(begin);
        int loadidx = 0;
        for (int index = 0; index < ehdr->e_phnum; ++index) {
            if (phdr[index].p_type != PT_LOAD)
                continue;

            uint32_t current = begin_block->vaddr() + RoundDown(phdr[index].p_vaddr, phdr[index].p_align);
            LoadBlock* block = api->findLoadBlock(current);

            if (!block)
                continue;

            if (current != block->vaddr())
                break;

            if (!(phdr[index].p_flags & PF_W) && !(block->flags() & PF_W)
                    && !(loadidx && !phdr[index].p_offset)) {
                uint32_t page_offset = RoundDown(phdr[index].p_offset, 0x1000);
                block->setMmapFile(file, page_offset);
            }
            ++loadidx;
        }
        return true;
    }
    return false;
}

uint32_t lp32::Core::dlsym32(const char* path, const char* symbol) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(path));
    if (map) {
        // already check valid on dlopen
        Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
        Elf32_Phdr* phdr = reinterpret_cast<Elf32_Phdr*>(map->data() + ehdr->e_phoff);

        int symtabndx = -1;
        int strtabndx = -1;

        int sh_num = ehdr->e_shnum;
        Elf32_Shdr* shdr = reinterpret_cast<Elf32_Shdr*>(map->data() + ehdr->e_shoff);
        const char* shstr = reinterpret_cast<const char*>(map->data() + shdr[ehdr->e_shstrndx].sh_offset);
        for (int i = 0; i < sh_num; ++i) {
            if (!strcmp(shstr + shdr[i].sh_name, ".symtab"))
                symtabndx = i;

            if (!strcmp(shstr + shdr[i].sh_name, ".strtab"))
                strtabndx = i;
        }

        if (symtabndx < 0 || strtabndx < 0)
            return 0x0;

        int count = shdr[symtabndx].sh_size / shdr[symtabndx].sh_entsize;
        Elf32_Sym* symtab = reinterpret_cast<Elf32_Sym*>(map->data() + shdr[symtabndx].sh_offset);
        const char* strtab = reinterpret_cast<const char*>(map->data() + shdr[strtabndx].sh_offset);
        for (int i = 0; i < count; ++i) {
            if (!strcmp(strtab + symtab[i].st_name, symbol))
                return symtab[i].st_value;
        }
    }
    return 0x0;
}
