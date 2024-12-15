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
#include "lp32/core.h"
#include "base/utils.h"
#include "common/bit.h"
#include "command/fake/map/fake_map.h"
#include "command/fake/core/lp32/fake_core.h"
#include <linux/elf.h>
#include <unistd.h>
#include <getopt.h>
#include <memory>
#include <set>

static int SECOND_FAKE_LINK_MAP_PAGES = 1;
static int SECOND_FAKE_STRTAB_PAGES = 1;

static void CreateFakeLinkMap32(uint32_t fake_vma, uint32_t fake_link_map,
                                std::set<std::string>& libs) {
    LOGI("Create FAKE LINK32 MAP\n");
    int idx = 0;
    int num = libs.size();
    uint32_t entry_size = RoundUp(sizeof(lp32::LinkMap), 0x10);

    lp32::LinkMap fake;
    fake.addr = fake_vma,
    fake.name = 0x0;
    fake.ld = 0x0;
    fake.next = fake_link_map + (idx + 1) * entry_size;
    fake.prev = 0x0;
    CoreApi::Write(fake_link_map, (void *)&fake, sizeof(lp32::LinkMap));
    ++idx;

    for (const auto& lib : libs) {
        lp32::LinkMap link;
        link.addr = FakeLinkMap::FindModuleFromLoadBlock(lib.c_str());
        link.name = 0x0;
        link.ld = 0x0;
        link.next = 0x0;
        link.prev = 0x0;
        if (idx < num) link.next = fake_link_map + (idx + 1) * entry_size;
        link.prev = fake_link_map + (idx - 1) * entry_size;
        CoreApi::Write(fake_link_map + idx * entry_size, (void *)&link, sizeof(lp32::LinkMap));
        ++idx;
    }
}

int FakeLinkMap::AutoCreate32() {
    std::set<std::string> libs;
    uint32_t link_map_len = 0x0;
    uint32_t strtab_len = 0x0;
    auto callback = [&](LoadBlock *block) -> bool {
        if (block->flags() & Block::FLAG_X
                && block->filename().length() > 0
                && block->filename()[0] == '/') {
            libs.insert(block->filename());
            link_map_len += RoundUp(sizeof(lp32::LinkMap), 0x10);
            strtab_len += RoundUp(block->filename().length() + 1, 0x10);
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, false, false);

    if (!libs.size()) {
        LOGW("Cannot found NT_FILE!\n");
        return 0;
    }

    uint32_t SECOND_FAKE_LINK_MAP_PAGES = RoundUp(link_map_len, CoreApi::GetPageSize()) / CoreApi::GetPageSize();
    uint32_t SECOND_FAKE_STRTAB_PAGES = RoundUp(strtab_len, CoreApi::GetPageSize()) / CoreApi::GetPageSize();

    uint32_t fake_vma = CoreApi::NewLoadBlock(0x0, CoreApi::GetPageSize() *
                    (FakeCore::FAKE_PHDR_PAGES + FakeCore::FKAE_DYNAMIC_PAGES
                   + SECOND_FAKE_LINK_MAP_PAGES + SECOND_FAKE_STRTAB_PAGES));
    if (!fake_vma) {
        LOGE("Create FAKE VMA fail.\n");
        return 0;
    }

    uint32_t fake_phdr = fake_vma;
    uint32_t fake_dynamic = fake_vma + CoreApi::GetPageSize() * FakeCore::FAKE_PHDR_PAGES;
    uint32_t fake_link_map = fake_dynamic + CoreApi::GetPageSize() * FakeCore::FKAE_DYNAMIC_PAGES;
    uint32_t fake_strtab = fake_link_map + CoreApi::GetPageSize() * SECOND_FAKE_LINK_MAP_PAGES;

    /** FAKE PHDR */
    lp32::FakeCore::CreateFakePhdr(fake_phdr, fake_dynamic);

    /** FAKE DYNAMIC */
    lp32::FakeCore::CreateFakeDynamic(fake_dynamic, fake_link_map);

    /** FAKE LINK MAP */
    CreateFakeLinkMap32(fake_vma, fake_link_map, libs);

    /** FAKE STRTAB */
    lp32::FakeCore::CreateFakeStrtab(fake_strtab, fake_link_map, libs);

    CoreApi::CleanCache();
    return 0;
}

int FakeLinkMap::Append32(uint32_t addr, const char* name) {
    return 0;
}

bool FakeLinkMap::FakeLD32(LinkMap* map) {
    LoadBlock* block = map->block();
    if (!block || !block->isMmapBlock())
        return false;

    Elf32_Ehdr *ehdr = reinterpret_cast<Elf32_Ehdr *>(block->begin(Block::OPT_READ_MMAP));
    Elf32_Phdr *phdr = reinterpret_cast<Elf32_Phdr *>(block->begin(Block::OPT_READ_MMAP) + ehdr->e_phoff);

    for (int num = 0; num < ehdr->e_phnum; ++num) {
        if (phdr[num].p_type == PT_PHDR) {
            if (phdr[num].p_offset != phdr[num].p_vaddr) {
                uint32_t vaddr = map->l_addr() + phdr[num].p_offset - phdr[num].p_vaddr;
                CoreApi::Write(map->Ptr() + OFFSET(LinkMap, l_addr), (void *)&vaddr, 4);
                LOGI("calibrate %s l_addr(%x)\n", map->name(), vaddr);
            }
            continue;
        }

        if (phdr[num].p_type == PT_DYNAMIC) {
            uint32_t vaddr = map->l_addr() + phdr[num].p_vaddr;
            if (map->l_ld() != vaddr) {
                CoreApi::Write(map->Ptr() + OFFSET(LinkMap, l_ld), (void *)&vaddr, 4);
                LOGI("calibrate %s l_ld(%x)\n", map->name(), vaddr);
            }
            break;
        }
    }
    return true;
}
