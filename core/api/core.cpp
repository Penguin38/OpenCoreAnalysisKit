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
#include "arm64/core.h"
#include "arm/core.h"
#include "riscv64/core.h"
#include "x64/core.h"
#include "x86/core.h"
#include "common/elf.h"
#include "common/exception.h"
#include "base/utils.h"
#include <linux/elf.h>
#include <cstring>
#include <iomanip>
#include <iostream>

CoreApi* CoreApi::INSTANCE = nullptr;

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
                INSTANCE = new x64::Core(map);
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
    uint64_t point_size = GetPointSize();
    return ((1ULL << (point_size - 1)) - 1) | (1ULL << (point_size - 1));
}

uint64_t CoreApi::GetVabitsMask() {
    return INSTANCE->getVabitsMask();
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

void CoreApi::addLoadBlock(std::shared_ptr<LoadBlock>& block) {
    mLoad.push_back(std::move(block));
}

LoadBlock* CoreApi::findLoadBlock(uint64_t vaddr) {
    for (const auto& block : mLoad) {
        if (block->virtualContains(vaddr)) {
            return block.get();
        }
    }
    return nullptr;
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

void CoreApi::addLinkMap(uint64_t begin, uint64_t name) {
    std::unique_ptr<LinkMap> linkmap;
    if (IsVirtualValid(name)) {
        linkmap = std::make_unique<LinkMap>(begin,
                            reinterpret_cast<const char*>(v2r(name, OPT_READ_ALL)),
                            findLoadBlock(begin));
    } else {
        linkmap = std::make_unique<LinkMap>(begin, nullptr, findLoadBlock(begin));
    }
    mLinkMap.push_back(std::move(linkmap));
}

void CoreApi::removeAllLinkMap() {
    mLinkMap.clear();
}

uint64_t CoreApi::GetDebug() {
    if (!INSTANCE->mDebug)
        INSTANCE->loadDebug();
    return INSTANCE->mDebug;
}

void CoreApi::DumpFile() {
    auto callback = [](File* file) -> bool {
        LOGI("[%lx, %lx) %lx %s\n", file->begin(), file->end(),
                                    file->offset(), file->name().c_str());
        return false;
    };
    INSTANCE->foreachFile(callback);
}

void CoreApi::ForeachFile(std::function<bool (File *)> callback) {
    INSTANCE->foreachFile(callback);
}

void CoreApi::DumpAuxv() {
    auto callback = [](Auxv* auxv) -> bool {
        if (auxv->type() == AT_EXECFN || auxv->type() == AT_PLATFORM) {
            std::string name;
            if (IsVirtualValid(auxv->value())) {
                name = reinterpret_cast<const char*>(GetReal(auxv->value()));
            }
            LOGI("%6lx  %16s  0x%lx %s\n", auxv->type(), auxv->to_string().c_str(),
                                            auxv->value(), name.c_str());
        } else {
            LOGI("%6lx  %16s  0x%lx\n", auxv->type(), auxv->to_string().c_str(), auxv->value());
        }
        return false;
    };
    INSTANCE->foreachAuxv(callback);
}

void CoreApi::DumpLinkMap() {
    auto callback = [](LinkMap* map) -> bool {
        LoadBlock* block = map->block();
        if (block) {
            std::string name;
            if (block->isMmapBlock()) {
                name = block->name();
            } else {
                name = map->name();
            }
            std::string valid;
            if (block->isValid()) {
                valid.append("[*]");
                if (block->isOverlayBlock()) {
                    valid.append("(OVERLAY)");
                } else if (block->isMmapBlock()) {
                    valid.append("(MMAP)");
                }
            } else {
                valid.append("[EMPTY]");
            }

            LOGI("[%lx, %lx) %s %s\n", block->vaddr(), block->vaddr() + block->size(),
                                       name.c_str(), valid.c_str());
        } else {
            LOGI("[%lx, %lx) %s [unknown]\n", block->vaddr(), block->vaddr() + block->size(),
                                       block->name().c_str());
        }
        return false;
    };
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
    uint64_t execfn = FindAuxv(AT_EXECFN);
    if (IsVirtualValid(phdr) && IsVirtualValid(execfn)) {
        std::string filepath;
        std::string search = reinterpret_cast<const char*>(GetReal(execfn));
        for (char *dir : dirs) {
            if (Utils::SearchFile(dir, &filepath, search))
                break;
        }
        if (filepath.length() > 0) {
            INSTANCE->sysroot(phdr, filepath.c_str());
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
        std::string filepath;
        for (char *dir : dirs) {
            if (Utils::SearchFile(dir, &filepath, map->name()))
                break;
        }
        if (filepath.length() > 0) {
            INSTANCE->sysroot(map->begin(), filepath.c_str());
        }
        return false;
    };
    INSTANCE->foreachLinkMap(callback);
}

void CoreApi::Write(uint64_t vaddr, uint64_t value) {
    LoadBlock* block = FindLoadBlock(vaddr);
    block->setOverlay(vaddr, value);
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

void CoreApi::ForeachLoadBlock(std::function<bool (LoadBlock *)> callback) {
    INSTANCE->foreachLoadBlock(callback);
}

LoadBlock* CoreApi::FindLoadBlock(uint64_t vaddr, bool check) {
    LoadBlock* block = INSTANCE->findLoadBlock(vaddr);
    if (check && block && block->isValid())
        return block;
    throw InvalidAddressException(vaddr);
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

void CoreApi::foreachLoadBlock(std::function<bool (LoadBlock *)> callback) {
    for (const auto& block : mLoad) {
        if (block->isValid()) {
            if (callback(block.get()))
                break;
        }
    }
}
