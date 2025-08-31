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
#include "api/core.h"
#include "common/bit.h"
#include "command/env.h"
#include "command/fake/map/fake_map.h"
#include "command/fake/exec/fake_executable.h"
#include "command/fake/core/arm64/fake_core.h"
#include "base/memory_map.h"
#include <linux/elf.h>

namespace arm64 {

struct user_pac_mask {
    uint64_t data_mask;
    uint64_t insn_mask;
};

int FakeCore::execute(const char* output) {
    std::unique_ptr<FakeCore::Stream>& stream = GetInputStream();

    if (!stream->Parse()) {
        LOGE("parse fakecore stream fail!\n");
        return 0;
    }

    has_pac = (stream->PacEnabledKeys() != -1);
    has_taggle_addr = (stream->TaggedAddr() != -1);
    std::vector<Opencore::VirtualMemoryArea>& maps = stream->Maps();

    Prepare(output);
    ParseProcessMapsVma(maps);
    CreateCoreHeader();
    CreateCoreNoteHeader();
    CreateCorePrStatus();
    CreateCoreAUXV();

    if (has_pac || has_taggle_addr) {
        auxv[use_auxv_num++].init(AT_HWCAP, 0xFFFFFFFF);
        auxv[use_auxv_num++].init(AT_HWCAP2, 0xFFFFFFFF);
        if (!va_bits && maps.size()) {
            int bits;
            uint64_t last_addr = maps[maps.size() - 1].end;
            for (bits = 32; bits < 64; bits++) {
                if (last_addr <= GENMASK_UL(bits - 1, 0))
                    break;
            }
            if (bits <= 54) {
                va_bits = bits;
                LOGI("vabits %d\n", va_bits);
            }
        }
    }

    ClocNoteFileSize();

    std::unique_ptr<MemoryMap> map(MemoryMap::MmapZeroMem(note.p_offset + note.p_filesz));
    if (!map) {
        LOGE("alloc size(0x%" PRIx64 ") fail, no vma!!\n", note.p_offset + note.p_filesz);
        return 0;
    }

    current_offset += WriteCoreHeader(map, current_offset);
    current_offset += WriteCoreNoteHeader(map, current_offset);
    current_offset += WriteCoreProgramHeaders(map, current_offset);

    current_offset += WriteCorePrStatus(map, current_offset);
    current_offset += WriteCoreAUXV(map, current_offset);
    current_offset += WriteNtFile(map, current_offset, maps);

    if (!CoreApi::Load(map, false, Env::Init))
        return 0;

    if (stream->Libs().size()) {
        /** core ready page_size == CoreApi::GetPageSize(); */
        uint64_t fake_vma = CoreApi::NewLoadBlock(0x0, page_size *
                (FAKE_PHDR_PAGES + FKAE_DYNAMIC_PAGES + FAKE_LINK_MAP_PAGES + FAKE_STRTAB_PAGES));
        if (!fake_vma) {
            LOGE("Create FAKE VMA fail.\n");
            return 0;
        }

        uint64_t fake_phdr = fake_vma;
        uint64_t fake_dynamic = fake_vma + page_size * FAKE_PHDR_PAGES;
        uint64_t fake_link_map = fake_dynamic + page_size * FKAE_DYNAMIC_PAGES;
        uint64_t fake_strtab = fake_link_map + page_size * FAKE_LINK_MAP_PAGES;

        /** FAKE PHDR */
        CreateFakePhdr(fake_phdr, fake_dynamic);

        /** FAKE DYNAMIC */
        CreateFakeDynamic(fake_dynamic, fake_link_map);

        /** FAKE LINK MAP */
        CreateFakeLinkMap(fake_vma, fake_link_map, stream->Libs(), maps);

        /** FAKE STRTAB */
        CreateFakeStrtab(fake_strtab, fake_link_map, stream->Libs());
    } else {
        FakeLinkMap::AutoCreate64();
    }

    /** LINKER LD */
    if (GetSysRootDir().length() > 0) {
        FakeLinkMap::SysRoot(GetSysRootDir().c_str());
        CoreApi::SysRoot(GetSysRootDir().c_str());
    }

    /** LLDB NEED AT_PHDR */
    if (NoFakePhdr()) {
        std::string executable = stream->Executable();
        if (GetExecutable().length())
            executable = GetExecutable();

        if (executable.length()) {
            LinkMap* handle = CoreApi::FindLinkMap(executable.c_str());
            if (handle)
                FakeExecutable::RebuildExecDynamic(handle);
            else
                LOGE("No found link_map %s!\n", executable.c_str());
        }
    }

    /** ORIGIN MEMORY CONTENT */
    std::map<uint64_t, uint64_t>& memorys = stream->Memorys();
    for (const auto& mem : memorys) {
        try {
            LoadBlock *block = CoreApi::FindLoadBlock(mem.first, false, false);
            if (block) {
                std::string& vma = block->filename();
                if (vma == "[anon:low shadow]"
                        || vma == "[anon:high shadow]"
                        || (vma.compare(0, 12, "[anon:hwasan") == 0))
                    continue;
                CoreApi::Write(mem.first, mem.second);
            }
        } catch(InvalidAddressException& e) {
            // do nothing
        }
    }

    /** CREATE FAKECORE */
    if (NeedRebuild()) {
        std::unique_ptr<::FakeCore> impl = FakeCore::Make(CoreApi::Bits());
        return impl ? impl->execute(output) : 0;
    }
    return 0;
}

void FakeCore::CreateCorePrStatus() {
    std::unique_ptr<FakeCore::Stream>& stream = GetInputStream();

    prnum = DEF_FAKE_PRNUM;
    prstatus = (Elf64_prstatus *)malloc(prnum * sizeof(Elf64_prstatus));
    memset(prstatus, 0, prnum * sizeof(Elf64_prstatus));

    pid_t tid = stream->Tid();
    prstatus[0].pr_pid = tid;
    memcpy((void *)&prstatus[0].pr_reg, stream->Regs(), sizeof(struct pt_regs));

    extra_note_filesz += (sizeof(Elf64_prstatus) + sizeof(Elf64_Nhdr) + 8) * prnum;

    if (HasPAC()) {
        extra_note_filesz += ((sizeof(struct user_pac_mask) + sizeof(Elf64_Nhdr) + 8)  // NT_ARM_PAC_MASK
                          + (sizeof(uint64_t) + sizeof(Elf64_Nhdr) + 8)       // NT_ARM_PAC_ENABLED_KEYS
                          ) * prnum;
    }

    if (HasTaggleAddr())
        extra_note_filesz += (sizeof(uint64_t) + sizeof(Elf64_Nhdr) + 8) * prnum;  // NT_ARM_TAGGED_ADDR_CTRL
}

uint64_t FakeCore::WriteCorePrStatus(std::unique_ptr<MemoryMap>& map, uint64_t off) {
    uint64_t tmp_off = 0x0;

    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_CORE_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(Elf64_prstatus);
    elf_nhdr.n_type = NT_PRSTATUS;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_CORE_NAME_SZ, ELFCOREMAGIC);

    int index = 0;
    while (index < prnum) {
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&elf_nhdr, sizeof(Elf64_Nhdr));
        tmp_off += sizeof(Elf64_Nhdr);
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)magic, sizeof(magic));
        tmp_off += sizeof(magic);
        memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&prstatus[index], sizeof(Elf64_prstatus));
        tmp_off += sizeof(Elf64_prstatus);

        if (HasPAC()) tmp_off += WriteCorePAC(index, map, off + tmp_off);
        if (HasTaggleAddr()) tmp_off += WriteCoreTaggleAddr(prstatus[index].pr_pid ,map, off + tmp_off);
        index++;
    }

    return tmp_off;
}

uint64_t FakeCore::WriteCorePAC(int, std::unique_ptr<MemoryMap>& map, uint64_t off) {
    uint64_t tmp_off = 0x0;
    // NT_ARM_PAC_MASK
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_LINUX_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(user_pac_mask);
    elf_nhdr.n_type = NT_ARM_PAC_MASK;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_LINUX_NAME_SZ, ELFLINUXMAGIC);

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&elf_nhdr, sizeof(Elf64_Nhdr));
    tmp_off += sizeof(Elf64_Nhdr);
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)magic, sizeof(magic));
    tmp_off += sizeof(magic);

    user_pac_mask uregs;
    uint64_t mask = GENMASK_UL(54, va_bits ? va_bits : DEF_VA_BITS);
    uregs.data_mask = mask;
    uregs.insn_mask = mask;

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&uregs, sizeof(user_pac_mask));
    tmp_off += sizeof(user_pac_mask);

    // NT_ARM_PAC_ENABLED_KEYS
    elf_nhdr.n_descsz = sizeof(uint64_t);
    elf_nhdr.n_type = NT_ARM_PAC_ENABLED_KEYS;

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&elf_nhdr, sizeof(Elf64_Nhdr));
    tmp_off += sizeof(Elf64_Nhdr);
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)magic, sizeof(magic));
    tmp_off += sizeof(magic);

    std::unique_ptr<FakeCore::Stream>& stream = GetInputStream();
    uint64_t pac_enabled_keys = stream->PacEnabledKeys();

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&pac_enabled_keys, sizeof(uint64_t));
    tmp_off += sizeof(uint64_t);

    return tmp_off;
}

uint64_t FakeCore::WriteCoreTaggleAddr(int, std::unique_ptr<MemoryMap>& map, uint64_t off) {
    uint64_t tmp_off = 0x0;

    // NT_ARM_TAGGED_ADDR_CTRL
    Elf64_Nhdr elf_nhdr;
    elf_nhdr.n_namesz = NOTE_LINUX_NAME_SZ;
    elf_nhdr.n_descsz = sizeof(uint64_t);
    elf_nhdr.n_type = NT_ARM_TAGGED_ADDR_CTRL;

    char magic[8];
    memset(magic, 0, sizeof(magic));
    snprintf(magic, NOTE_LINUX_NAME_SZ, ELFLINUXMAGIC);

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&elf_nhdr, sizeof(Elf64_Nhdr));
    tmp_off += sizeof(Elf64_Nhdr);
    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)magic, sizeof(magic));
    tmp_off += sizeof(magic);

    std::unique_ptr<FakeCore::Stream>& stream = GetInputStream();
    uint64_t tagged_addr_ctrl = stream->TaggedAddr();

    memcpy(reinterpret_cast<void *>(map->data() + off + tmp_off), (void *)&tagged_addr_ctrl, sizeof(uint64_t));
    tmp_off += sizeof(uint64_t);

    return tmp_off;
}

FakeCore::~FakeCore() {
    if (prstatus) free(prstatus);
}

} // namespace arm64
