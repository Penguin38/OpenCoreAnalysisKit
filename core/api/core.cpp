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

#include "api/core.h"
#include "arm64/core.h"
#include "arm/core.h"
#include "riscv64/core.h"
#include "x64/core.h"
#include "x86/core.h"
#include "common/elf.h"
#include "common/exception.h"
#include <linux/elf.h>
#include <string.h>
#include <memory>
#include <iostream>

CoreApi* CoreApi::INSTANCE = nullptr;

bool CoreApi::Load(const char* corefile) {
    UnLoad();
    if (!INSTANCE) {
        std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(corefile));
        if (map) {
            ElfHeader* header = reinterpret_cast<ElfHeader*>(map->data());
            if (memcmp(header->ident, ELFMAG, 4)) {
                std::cout << "Invalid ELF file." << std::endl;
                return false;
            }
    
            if (header->type != ET_CORE) {
                std::cout << "Invalid Core file." << std::endl;
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
                    std::cout << "Not support machine (" << header->machine << ")" << std::endl;
                    break;
            }

            if (INSTANCE) {
                return INSTANCE->load();
            }
        }
    }
    return false;
}

void CoreApi::UnLoad() {
    if (INSTANCE) {
        INSTANCE->unload();
        INSTANCE->removeAllLoadBlock();
        delete INSTANCE;
        INSTANCE = nullptr;
        std::cout << __func__ << std::endl;
    }
}

const char* CoreApi::GetMachine() {
    if (INSTANCE) {
        return INSTANCE->getMachine();
    }
    return "unknown";
}

CoreApi::~CoreApi() {
    mCore.reset();
    std::cout << __func__ << " " << this << std::endl;
}

uint64_t CoreApi::begin() {
    return reinterpret_cast<uint64_t>(mCore->data());
}

void CoreApi::addLoadBlock(std::unique_ptr<LoadBlock>& block) {
    mLoad.push_back(std::move(block));
}

void CoreApi::removeAllLoadBlock() {
    mLoad.clear();
}

uint64_t CoreApi::GetReal(uint64_t vaddr) {
    return INSTANCE->v2r(vaddr);
}

uint64_t CoreApi::GetVirtual(uint64_t raddr) {
    return INSTANCE->r2v(raddr);
}

uint64_t CoreApi::v2r(uint64_t vaddr) {
    for (const auto& block : mLoad) {
        if (block->virtualContains(vaddr) && block->isValid())
            return block->begin();
    }
    throw InvalidAddressException(vaddr);
}

uint64_t CoreApi::r2v(uint64_t raddr) {
    for (const auto& block : mLoad) {
        if (block->realContains(raddr))
            return block->vaddr();
    }
    throw InvalidAddressException(raddr);
}
