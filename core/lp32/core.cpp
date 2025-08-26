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
#include "zip/zip_file.h"
#include "common/elf.h"
#include "common/bit.h"
#include "common/load_block.h"
#include "common/exception.h"
#include "common/xz/codec.h"
#include <string.h>
#include <linux/elf.h>

bool lp32::Core::load32(CoreApi* api, std::function<void* (OptionArgs&)> callback) {
    Elf32_Ehdr *ehdr = reinterpret_cast<Elf32_Ehdr *>(api->begin());
    Elf32_Phdr *phdr = reinterpret_cast<Elf32_Phdr *>(api->begin() + sizeof(Elf32_Ehdr));

    for (int num = 0; num < ehdr->e_phnum; ++num) {
        if (phdr[num].p_type == PT_LOAD) {
            std::shared_ptr<LoadBlock> block(new LoadBlock(phdr[num].p_flags,
                                             phdr[num].p_offset,
                                             phdr[num].p_vaddr,
                                             phdr[num].p_paddr,
                                             phdr[num].p_filesz,
                                             phdr[num].p_memsz,
                                             phdr[num].p_align));
            block->setTruncated(api->size() < (block->offset() + block->realSize()));
            block->setOriAddr(api->begin() + block->offset());
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
            block->setTruncated(api->size() < (block->offset() + block->realSize()));
            if (!block->isValidBlock())
                continue;

            block->setOriAddr(api->begin() + block->offset());
            uint64_t pos = block->oraddr();
            uint64_t end = block->oraddr() + block->realSize();
            void *last_context = nullptr;
            while (pos < end) {
                Elf32_Nhdr *nhdr = reinterpret_cast<Elf32_Nhdr *>(pos);
                uint64_t item_pos = nhdr->n_descsz == 0 ? 0 : pos + sizeof(Elf32_Nhdr) + RoundUp(nhdr->n_namesz, 0x4);
                OptionArgs args = {
                    .type = nhdr->n_type,
                    .pos = item_pos,
                    .context = last_context,
                };
                switch(nhdr->n_type) {
                    case NT_PRSTATUS: {
                        last_context = callback(args);
                        block->addThreadItem(last_context);
                    } break;
                    case NT_AUXV: {
                        int numauxv = nhdr->n_descsz / sizeof(lp32::Auxv);
                        block->setAuxvMaxCount(numauxv);
                        lp32::Auxv* auxv = reinterpret_cast<lp32::Auxv *>(item_pos);
                        for (int index = 0; index < numauxv; ++index) {
                            block->addAuxvItem((uint64_t)&auxv[index], auxv[index].type, auxv[index].value);
                            if (!auxv[index].type) break;
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
                    default:
                        callback(args);
                        break;
                }

                pos = pos + sizeof(Elf32_Nhdr) + RoundUp(nhdr->n_namesz, 0x4) + RoundUp(nhdr->n_descsz, 0x4);
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
        LOGW("Not found debug. You can try command exec.\n");
        return;
    }

    try {
        lp32::Debug* dbg = reinterpret_cast<lp32::Debug*>(debug.Real());
        uint32_t map = dbg->map;
        while (map) {
            lp32::LinkMap* link = reinterpret_cast<lp32::LinkMap*>(CoreApi::GetReal(map));
            api->addLinkMap(map);
            map = link->next;
        }
    } catch (InvalidAddressException& e) {
        LOGE("%s\n", e.what());
    }
}

bool lp32::Core::exec32(CoreApi* api, uint32_t phdr, const char* file) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(file));
    if (map) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
        if (!header->CheckLibrary(file))
            return false;

        Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
        Elf32_Phdr* pt = reinterpret_cast<Elf32_Phdr*>(map->data() + sizeof(Elf32_Ehdr));

        if (pt[0].p_type != PT_PHDR) {
            LOGE("Exec file PHDR segment non-first\n");
            return false;
        }

        return loader_dlopen32(api, map.get(), nullptr, phdr - ehdr->e_phoff - (pt[0].p_vaddr - pt[0].p_offset), file);
    }
    return false;
}

bool lp32::Core::dlopen32(CoreApi* api, ::LinkMap* handle, const char* file, const char* subfile) {
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(file));
    if (subfile) {
        ZipFile zip;
        if (zip.open(file)) {
            LOGE("Zip open fail %s\n", file);
            return false;
        }

        ZipEntry* entry;
        if (subfile[0] == '/') {
            entry = zip.getEntryByName(subfile + 1);
        } else {
            entry = zip.getEntryByName(subfile);
        }
        if (!entry) {
            LOGE("%s Not found entry %s\n", file, subfile);
            return false;
        }

        if (!entry->IsUncompressed()) {
            LOGE("Not support compress zip %s!%s\n", file, entry->getFileName());
            return false;
        }

        std::unique_ptr<MemoryMap> submap(MemoryMap::MmapFile(file,
                                                              entry->getEntryTotalMemsz(),
                                                              entry->getFileOffset()));
        map = std::move(submap);
    }
    if (map) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
        if (!header->CheckLibrary(file))
            return false;

        return loader_dlopen32(api, map.get(), handle, handle->l_addr(), file);
    }
    return false;
}

bool lp32::Core::loader_dlopen32(CoreApi* api, MemoryMap* map, ::LinkMap* handle, uint32_t addr, const char* file) {
    if (!addr) {
        LOGE("Can not loader_dlopen32 %s\n", file);
        return false;
    }

    bool status = false;
    Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
    Elf32_Phdr* phdr = reinterpret_cast<Elf32_Phdr*>(map->data() + sizeof(Elf32_Ehdr));

    int idx = 0;
    for (int index = 0; index < ehdr->e_phnum; ++index) {
        if (phdr[index].p_type != PT_LOAD)
            continue;
        ++idx;

        if (phdr[index].p_flags & PF_W)
            continue;

        if (!phdr[index].p_filesz)
            continue;

        uint32_t current = addr + RoundDown(phdr[index].p_vaddr, CoreApi::GetPageSize());
        uint32_t page_offset = RoundDown(phdr[index].p_offset + map->offset(), CoreApi::GetPageSize());

        LoadBlock* block = api->findLoadBlock(current, false);
        if (!block) {
            LOGW("Not found %s [%d]LOAD(%x)\n", file, idx, current);
            continue;
        }

        if (idx == 1 && current == block->vaddr() && handle)
            handle->GetAddrCache() = current;

        if (!block->CheckCanMmap(current))
            continue;

        ::File* vma = CoreApi::FindFile(current);
        if (vma && page_offset != vma->offset()) {
            page_offset = vma->offset();
        }

        uint32_t mem_size = RoundUp(phdr[index].p_offset + map->offset()
                                  + phdr[index].p_memsz, phdr[index].p_align);
        if (mem_size < page_offset)
            return status;
        mem_size -= page_offset;
        if (mem_size >= block->memsz()) {
            block->setMmapFile(file, page_offset);
            if (handle) block->bind(handle);
            status = true;
        } else {
            LOGE("Please checksum %s miss match.\n", file);
            break;
        }

        // continue mmap
        if (mem_size > block->memsz()) {
            LOGD("Mmap segment [%" PRIx64 ", %" PRIx64 ") size %" PRIx64 " != %x, maybe reset range!\n",
                    block->vaddr(), block->vaddr() + block->memsz(), block->memsz(), mem_size);

            uint32_t cur_size = block->memsz();
            while (cur_size < mem_size) {
                uint32_t next = current + cur_size;
                uint32_t next_page_offset = page_offset + cur_size;

                LoadBlock* next_block = api->findLoadBlock(next, false);
                if (!next_block) {
                    LOGD("Not found next LoadBlock(%x)\n", next);
                    break;
                }
                cur_size += next_block->memsz();

                if (!next_block->CheckCanMmap(next))
                    continue;

                ::File* next_vma = CoreApi::FindFile(next);
                if (next_vma && next_page_offset != next_vma->offset()) {
                    next_page_offset = next_vma->offset();
                }
                next_block->setMmapFile(file, next_page_offset);
                if (handle) next_block->bind(handle);
            }
        }
    }
    return status;
}

static void ReadSymbolEntry32(std::unique_ptr<MemoryMap>& map, int symndx, int strndx,
                            std::unordered_set<SymbolEntry, SymbolEntry::Hash>& symbols) {
    if (!symndx || !strndx)
        return;

    Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
    Elf32_Shdr* shdr = reinterpret_cast<Elf32_Shdr*>(map->data() + ehdr->e_shoff);

    int count = shdr[symndx].sh_size / shdr[symndx].sh_entsize;
    Elf32_Sym* symtab = reinterpret_cast<Elf32_Sym*>(map->data() + shdr[symndx].sh_offset);
    const char* strtab = reinterpret_cast<const char*>(map->data() + shdr[strndx].sh_offset);
    for (int i = 0; i < count; ++i) {
        if (symtab[i].st_value && symtab[i].st_size) {
            SymbolEntry entry = SymbolEntry(symtab[i].st_value, symtab[i].st_info, symtab[i].st_size,
                    const_cast<const char*>(strtab + symtab[i].st_name));
            symbols.insert(entry);
        }
    }
}

static void ReadSymbol32(std::unique_ptr<MemoryMap>& map,
                         std::unordered_set<SymbolEntry, SymbolEntry::Hash>& symbols) {
    if (!map) return;

    int dynsymndx = 0;
    int dynstrndx = 0;
    int symtabndx = 0;
    int strtabndx = 0;

    // already check truncated
    Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
    Elf32_Shdr* shdr = reinterpret_cast<Elf32_Shdr*>(map->data() + ehdr->e_shoff);
    const char* shstr = reinterpret_cast<const char*>(map->data() + shdr[ehdr->e_shstrndx].sh_offset);

    int sh_num = ehdr->e_shnum;
    for (int i = 0; i < sh_num; ++i) {
        uint64_t addr = reinterpret_cast<uint64_t>(shstr + shdr[i].sh_name);
        if (addr < map->data() || addr >= (map->data() + map->size()))
            continue;

        if (shdr[i].sh_type == SHT_NOBITS)
            continue;

        if (!strcmp(shstr + shdr[i].sh_name, ".dynsym")) {
            dynsymndx = i;
            continue;
        }

        if (!strcmp(shstr + shdr[i].sh_name, ".dynstr")) {
            dynstrndx = i;
            continue;
        }

        if (!strcmp(shstr + shdr[i].sh_name, ".symtab")) {
            symtabndx = i;
            continue;
        }

        if (!strcmp(shstr + shdr[i].sh_name, ".strtab")) {
            strtabndx = i;
            continue;
        }
    }

    ReadSymbolEntry32(map, dynsymndx, dynstrndx, symbols);
    ReadSymbolEntry32(map, symtabndx, strtabndx, symbols);
}

void lp32::Core::readsym32(::LinkMap* handle) {
    if (!handle->block())
        return;

    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(handle->block()->name().c_str(), handle->block()->GetMmapOffset()));
    std::unordered_set<SymbolEntry, SymbolEntry::Hash>& symbols = handle->block()->GetSymbols();
    if (map) {
        // already check valid on dlopen
        int gnu_debugdatandx = 0;

        Elf32_Ehdr* ehdr = reinterpret_cast<Elf32_Ehdr*>(map->data());
        if (ehdr->e_shoff > map->size()) {
            LOGW("%s mmap file is truncated or no match.\n", map->getName().c_str());
            return;
        }

        Elf32_Shdr* shdr = reinterpret_cast<Elf32_Shdr*>(map->data() + ehdr->e_shoff);
        if (shdr[ehdr->e_shstrndx].sh_offset > map->size()) {
            LOGW("%s mmap file is truncated or no match.\n", map->getName().c_str());
            return;
        }

        const char* shstr = reinterpret_cast<const char*>(map->data() + shdr[ehdr->e_shstrndx].sh_offset);

        int sh_num = ehdr->e_shnum;
        for (int i = 0; i < sh_num; ++i) {
            uint64_t addr = reinterpret_cast<uint64_t>(shstr + shdr[i].sh_name);
            if (addr < map->data() || addr >= (map->data() + map->size()))
                continue;

            if (shdr[i].sh_type == SHT_NOBITS)
                continue;

            if (!strcmp(shstr + shdr[i].sh_name, ".gnu_debugdata")) {
                gnu_debugdatandx = i;
                break;
            }
        }

        ReadSymbol32(map, symbols);

        // scan gnu_debugdata
        if (gnu_debugdatandx > 0) {
            if (!xz::Codec::HasLZMASupport())
                return;

            std::unique_ptr<xz::Codec> codec = xz::Codec::Create(
                    reinterpret_cast<uint8_t *>(map->data() + shdr[gnu_debugdatandx].sh_offset),
                    shdr[gnu_debugdatandx].sh_size);

            if (!codec)
                return;

            std::unique_ptr<MemoryMap> debug_map(codec->Decode2Map());
            if (debug_map) {
                ElfHeader* header = reinterpret_cast<ElfHeader*>(debug_map->data());
                std::string anon_name = "anon:gnu_debugdata_";
                anon_name.append(map->getName());
                if (!header->CheckLibrary(anon_name.c_str()))
                    return;
                ReadSymbol32(debug_map, symbols);
            }
        }
    }
}
