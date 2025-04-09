/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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
#include "lp64/core.h"
#include "api/core.h"
#include "api/elf.h"
#include "common/elf.h"
#include "base/memory_map.h"
#include "command/fake/exec/fake_executable.h"
#include <linux/elf.h>
#include <memory>

bool FakeExecutable::RebuildExecDynamic64(LinkMap* executable) {
    if (!executable)
        return false;

    LoadBlock* block = executable->block();
    if (!block || !block->isMmapBlock()) {
        LOGE("Must mmap executable image %s!\n", executable->name());
        return false;
    }

    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(block->name().c_str()));
    if (map) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
        if (!header->CheckLibrary(map->getName().c_str()))
            return false;

        Elf64_Ehdr* ehdr = reinterpret_cast<Elf64_Ehdr*>(map->data());
        Elf64_Phdr* phdr = reinterpret_cast<Elf64_Phdr*>(map->data() + ehdr->e_phoff);

        if (phdr[0].p_type != PT_PHDR) {
            LOGE("Exec file PHDR segment non-first\n");
            return false;
        }

        // rebuild dynamic
        for (int i = 0; i < ehdr->e_phnum; ++i) {
            if (phdr[i].p_type != PT_DYNAMIC)
                continue;

            lp64::Dynamic *tmp_dynamic = (lp64::Dynamic *)(map->data() + phdr[i].p_offset);
            int numdynamic = phdr[i].p_filesz / sizeof(lp64::Dynamic);

            api::Elfx_Dynamic exec_dynamic = api::Elf::FindDynamic(executable);

            bool find_debug = false;
            int idx = 0;
            while (idx < numdynamic) {
                if (tmp_dynamic[idx].type == DT_DEBUG) {
                    find_debug = true;
                    break;
                }
                idx++;
            }

            if (!find_debug) {
                LOGE("Not found DT_DEBUG on %s\n", map->getName().c_str());
                return false;
            }

            idx = 0;
            while (idx < numdynamic) {
                CoreApi::Write(exec_dynamic.Ptr() + idx * sizeof(lp64::Dynamic),
                               (void *)&tmp_dynamic[idx], sizeof(lp64::Dynamic));
                if (tmp_dynamic[idx].type == DT_DEBUG) {
                    lp64::Dynamic dynamic;
                    dynamic.type = DT_DEBUG;
                    dynamic.value = CoreApi::GetDebugPtr();
                    CoreApi::Write(exec_dynamic.Ptr() + idx * sizeof(lp64::Dynamic),
                                   (void *)&dynamic, sizeof(lp64::Dynamic));
                }
                idx++;
            }
        }

        // AT_ENTRY
        uint64_t text_addr = 0;

        Elf64_Shdr* shdr = reinterpret_cast<Elf64_Shdr*>(map->data() + ehdr->e_shoff);
        const char* shstr = reinterpret_cast<const char*>(map->data() + shdr[ehdr->e_shstrndx].sh_offset);

        int sh_num = ehdr->e_shnum;
        for (int i = 0; i < sh_num; ++i) {
            uint64_t addr = reinterpret_cast<uint64_t>(shstr + shdr[i].sh_name);
            if (addr < map->data() || addr >= (map->data() + map->size()))
                continue;

            if (!strcmp(shstr + shdr[i].sh_name, ".text")) {
                text_addr = shdr[i].sh_addr;
                break;
            }
        }

        bool already_has_entry = false;
        auto fake_auxv_callback = [&](::Auxv* auxv) -> bool {
            if (auxv->type() == AT_PHDR) {
                auxv->setValue(executable->l_addr() + sizeof(Elf64_Ehdr));
            } else if (auxv->type() == AT_PHNUM) {
                auxv->setValue(ehdr->e_phnum);
            } else if (auxv->type() == AT_ENTRY) {
                already_has_entry = true;
                auxv->setValue(executable->l_addr() + text_addr);
            }
            return false;
        };
        CoreApi::ForeachAuxv(fake_auxv_callback);

        if (!already_has_entry) {
            std::vector<std::unique_ptr<NoteBlock>>& notes = CoreApi::GetNotes();
            for (const auto& note : notes) {
                if (!note->getAuxvMaxCount())
                    continue;
                note->appendAuxvItem(AT_ENTRY, executable->l_addr() + text_addr);
                break;
            }
        }
        return true;
    }
    return false;
}
