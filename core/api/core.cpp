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
#include "api/elf.h"
#include "arm64/core.h"
#include "arm/core.h"
#include "riscv64/core.h"
#include "x86_64/core.h"
#include "x86/core.h"
#include "common/elf.h"
#include "common/exception.h"
#include "base/utils.h"
#include "base/macros.h"
#include <linux/elf.h>
#include <cstring>
#include <iomanip>
#include <iostream>

CoreApi* CoreApi::INSTANCE = nullptr;

void CoreApi::Init() {
    api::Elf::Init();
    LinkMap::Init();
}

bool CoreApi::IsReady() {
    return INSTANCE != nullptr;
}

bool CoreApi::Load(const char* corefile) {
    UnLoad();
    std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(corefile));
    if (map) {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
        if (memcmp(header->ident, ELFMAG, 4)) {
            LOGI("Invalid ELF file.\n");
            return false;
        }

        if (header->type != ET_CORE) {
            LOGI("Invalid Core file.\n");
            return false;
        }

        switch(header->machine) {
            case EM_AARCH64:
                INSTANCE = new arm64::Core(map);
                break;
            case EM_ARM:
                INSTANCE = new arm::Core(map);
                break;
            case EM_RISCV:
                INSTANCE = new riscv64::Core(map);
                break;
            case EM_X86_64:
                INSTANCE = new x86_64::Core(map);
                break;
            case EM_386:
                INSTANCE = new x86::Core(map);
                break;
            default:
                LOGI("Not support machine (%d)\n", header->machine);
                break;
        }

        if (INSTANCE)
            return INSTANCE->load();
    }
    return false;
}

void CoreApi::UnLoad() {
    if (INSTANCE) {
        INSTANCE->unload();
        INSTANCE->removeAllLinkMap();
        INSTANCE->removeAllLoadBlock();
        INSTANCE->removeAllNoteBlock();
        delete INSTANCE;
        INSTANCE = nullptr;
    }
}

const char* CoreApi::GetName() {
    return INSTANCE->getName().c_str();
}

const char* CoreApi::GetMachineName() {
    return INSTANCE->getMachineName();
}

int CoreApi::GetMachine() {
    return INSTANCE->getMachine();
}

int CoreApi::GetPointSize() {
    return INSTANCE->getPointSize();
}

uint64_t CoreApi::GetPointMask() {
    return INSTANCE->pointer_mask;
}

uint64_t CoreApi::GetVabitsMask() {
    return INSTANCE->vabits_mask;
}

CoreApi::~CoreApi() {
    LOGI("Remove core (%p) %s\n", this, mCore->getName().c_str());
    mCore.reset();
}

uint64_t CoreApi::begin() {
    return mCore->data();
}

uint64_t CoreApi::size() {
    return mCore->size();
}

std::string& CoreApi::getName() {
    return mCore->getName();
}

bool CoreApi::NewLoadBlock(uint64_t begin, uint64_t size) {
    if (INSTANCE->findLoadBlock(begin)
            || INSTANCE->findLoadBlock(begin + size - 0x1))
        return false;

    std::shared_ptr<LoadBlock> block(new LoadBlock(Block::FLAG_R | Block::FLAG_W,  // flag
                                                   0x0,      // offset
                                                   begin,    // vaddr
                                                   0x0,      // paddr
                                                   0x0,      // filesz
                                                   size,     // memsz
                                                   0x1000)); // align
    block->setTruncated(false);
    block->setFake(true);
    block->setVabitsMask(CoreApi::GetVabitsMask());
    block->setPointMask(CoreApi::GetPointMask());

    if (!block->newOverlay())
        return false;

    INSTANCE->addLoadBlock(block);
    return true;
}

void CoreApi::addLoadBlock(std::shared_ptr<LoadBlock>& block) {
    mLoad.push_back(std::move(block));
}

void CoreApi::removeAllLoadBlock() {
    mLoad.clear();
}

uint64_t CoreApi::GetReal(uint64_t vaddr, int opt) {
    return INSTANCE->v2r(vaddr, opt);
}

uint64_t CoreApi::GetVirtual(uint64_t raddr) {
    return INSTANCE->r2v(raddr);
}

bool CoreApi::IsVirtualValid(uint64_t vaddr) {
    return INSTANCE->virtualValid(vaddr);
}

uint64_t CoreApi::FindAuxv(uint64_t type) {
    return INSTANCE->findAuxv(type);
}

ThreadApi* CoreApi::FindThread(int tid) {
    return INSTANCE->findThread(tid);
}

void CoreApi::addLinkMap(uint64_t map) {
    std::unique_ptr<LinkMap> linkmap = std::make_unique<LinkMap>(map);
    mLinkMap.push_back(std::move(linkmap));
}

void CoreApi::removeAllLinkMap() {
    mLinkMap.clear();
}

void CoreApi::Dump() {
    LOGI("Core env: %s\n", GetName());
    LOGI("  * Machine: %s\n", GetMachineName());
    LOGI("  * Bits: %d\n", Bits());
    LOGI("  * PointSize: %d\n", GetPointSize());
    LOGI("  * PointMask: 0x%lx\n", GetPointMask());
    LOGI("  * VabitsMask: 0x%lx\n", GetVabitsMask());
    LOGI("  * PageSize: 0x%lx\n", GetPageSize());
}

void CoreApi::ForeachFile(std::function<bool (File *)> callback) {
    INSTANCE->foreachFile(callback);
}

File* CoreApi::FindFile(uint64_t vaddr) {
    File *result = nullptr;
    if (!vaddr) return result;
    uint64_t clocvaddr = vaddr & GetVabitsMask();
    auto callback = [&](File* file) -> bool {
        if (file->contains(clocvaddr)) {
            result = file;
            return true;
        }
        return false;
    };
    INSTANCE->foreachFile(callback);
    return result;
}

void CoreApi::ForeachAuxv(std::function<bool (Auxv *)> callback) {
    INSTANCE->foreachAuxv(callback);
}

void CoreApi::ForeachLinkMap(std::function<bool (LinkMap *)> callback) {
    INSTANCE->foreachLinkMap(callback);
}

void CoreApi::ExecFile(const char* path) {
    std::vector<char *> dirs;
    std::unique_ptr<char> newpath(strdup(path));
    char *token = strtok(newpath.get(), ":");
    while (token != nullptr) {
        dirs.push_back(token);
        token = strtok(nullptr, ":");
    }

    uint64_t phdr = FindAuxv(AT_PHDR);
    api::MemoryRef execfn = FindAuxv(AT_EXECFN);
    if (execfn.IsValid()) {
        std::string filepath;
        const char* search = reinterpret_cast<const char*>(execfn.Real());
        for (char *dir : dirs) {
            if (Utils::SearchFile(dir, &filepath, search))
                break;
        }
        if (filepath.length() > 0) {
            INSTANCE->exec(phdr, filepath.c_str());
        }
    }
}

void CoreApi::SysRoot(const char* path) {
    std::vector<char *> dirs;
    std::unique_ptr<char> newpath(strdup(path));
    char *token = strtok(newpath.get(), ":");
    while (token != nullptr) {
        dirs.push_back(token);
        token = strtok(nullptr, ":");
    }

    auto callback = [dirs](LinkMap* map) -> bool {
        std::unique_ptr<char> newname(strdup(map->name()));
        char *ori_file = strtok(newname.get(), "!");
        char *sub_file = strtok(NULL, "!");

        std::string filepath;
        for (char *dir : dirs) {
            if (Utils::SearchFile(dir, &filepath, ori_file))
                break;
        }
        if (filepath.length() > 0) {
            if (INSTANCE->sysroot(map, filepath.c_str(), sub_file)) {
                map->ReadSymbols();
            }
        }
        return false;
    };
    INSTANCE->foreachLinkMap(callback);
}

void CoreApi::Write(uint64_t vaddr, void *buf, uint64_t size) {
    LoadBlock* block = FindLoadBlock(vaddr);
    block->setOverlay(vaddr, buf, size);
}

bool CoreApi::Read(uint64_t vaddr, uint64_t size, uint8_t* buf, int opt) {
    LoadBlock* block = FindLoadBlock(vaddr);

    uint64_t raddr = GetReal(vaddr, opt);
    if (!raddr)
        return false;

    if ((vaddr + size) > (block->vaddr() + block->size())) {
        uint64_t newsize = (block->vaddr() + block->size() - vaddr);
        memcpy(buf, reinterpret_cast<void *>(raddr), newsize);
    } else {
        memcpy(buf, reinterpret_cast<void *>(raddr), size);
    }
    return true;
}

void CoreApi::ForeachLoadBlock(std::function<bool (LoadBlock *)> callback, bool check) {
    INSTANCE->foreachLoadBlock(callback, check);
}

uint64_t CoreApi::DlSym(const char* path, const char* symbol) {
    LinkMap* handle = nullptr;
    auto callback = [&](LinkMap* map) -> bool {
        LoadBlock* block = map->block();
        if (block) {
            if (!strcmp(map->name(), path))
                handle = map;

            if (block->isMmapBlock()) {
                std::size_t index = block->name().find(path);
                if (index != std::string::npos)
                    handle = map;
            }

            if (handle) return true;
        }
        return false;
    };
    INSTANCE->foreachLinkMap(callback);

    if (handle) {
        LoadBlock* block = handle->block();
        if (block) {
            uint64_t value = handle->DlSym(symbol);
            if (value) return handle->l_addr() + value;
        }
    }
    return 0x0;
}

void CoreApi::ForeachThread(std::function<bool (ThreadApi *)> callback) {
    INSTANCE->foreachThread(callback);
}

uint64_t CoreApi::v2r(uint64_t vaddr, int opt) {
    for (const auto& block : mLoad) {
        if (block->virtualContains(vaddr) && block->isValid()) {
            uint64_t raddr = block->begin(opt);
            if (raddr) {
                return raddr + ((vaddr & block->VabitsMask()) - block->vaddr());
            } else {
                return 0x0;
            }
        }
    }
    throw InvalidAddressException(vaddr);
}

uint64_t CoreApi::r2v(uint64_t raddr) {
    for (const auto& block : mLoad) {
        if (block->realContains(raddr))
            return block->vaddr() + (raddr - block->begin());
    }
    throw InvalidAddressException(raddr);
}

bool CoreApi::virtualValid(uint64_t vaddr) {
    for (const auto& block : mLoad) {
        if (block->virtualContains(vaddr)) {
            return block->isValid();
        }
    }
    return false;
}

void CoreApi::addNoteBlock(std::unique_ptr<NoteBlock>& block) {
    mNote.push_back(std::move(block));
}

void CoreApi::removeAllNoteBlock() {
    mNote.clear();
}

uint64_t CoreApi::findAuxv(uint64_t type) {
    for (const auto& block : mNote) {
        for (const auto& auxv : block->getAuxv()) {
            if (auxv->type() == type)
                return auxv->value();
        }
    }
    return 0x0;
}

ThreadApi* CoreApi::findThread(int tid) {
    for (const auto& block : mNote) {
        for (const auto& thread : block->getThread()) {
            if (thread->pid() == tid)
                return thread.get();
        }
    }
    return nullptr;
}

void CoreApi::foreachThread(std::function<bool (ThreadApi *)> callback) {
    for (const auto& block : mNote) {
        for (const auto& thread : block->getThread()) {
            if (callback(thread.get()))
                return;
        }
    }
}

void CoreApi::foreachFile(std::function<bool (File *)> callback) {
    for (const auto& block : mNote) {
        for (const auto& file : block->getFile()) {
            if (callback(file.get()))
                return;
        }
    }
}

void CoreApi::foreachAuxv(std::function<bool (Auxv *)> callback) {
    for (const auto& block : mNote) {
        for (const auto& auxv : block->getAuxv()) {
            if (callback(auxv.get()))
                return;
        }
    }
}

void CoreApi::foreachLinkMap(std::function<bool (LinkMap *)> callback) {
    if (mLinkMap.size() == 0) {
        loadLinkMap();
    }

    for (const auto& map : mLinkMap) {
        if (callback(map.get()))
            break;
    }
}

void CoreApi::foreachLoadBlock(std::function<bool (LoadBlock *)> callback, bool check) {
    for (const auto& block : mLoad) {
        if (LIKELY(check) && !block->isValid())
            continue;

        if (callback(block.get()))
            break;
    }
}

uint64_t CoreApi::getPageSize() {
    if (!page_size) {
        page_size = findAuxv(AT_PAGESZ);
        if (!page_size) {
            LOGW("WARN: Use default page_size: %x\n", ELF_PAGE_SIZE);
            page_size = ELF_PAGE_SIZE;
        }
    }
    return page_size;
}
