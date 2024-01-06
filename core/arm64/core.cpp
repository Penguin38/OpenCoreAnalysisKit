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

#include "arm64/core.h"
#include <linux/elf.h>
#include <memory>
#include <iostream>

namespace arm64 {

bool Core::load() {
    Elf64_Ehdr *ehdr = reinterpret_cast<Elf64_Ehdr *>(begin());
    Elf64_Phdr *phdr = reinterpret_cast<Elf64_Phdr *>(begin() + ehdr->e_phoff);

    for (int num = 0; num < ehdr->e_phnum; ++num) {
        if (phdr[num].p_type == PT_LOAD) {
            std::shared_ptr<LoadBlock> block(new LoadBlock(phdr[num].p_flags,
                                             phdr[num].p_offset,
                                             phdr[num].p_vaddr,
                                             phdr[num].p_paddr,
                                             phdr[num].p_filesz,
                                             phdr[num].p_memsz,
                                             phdr[num].p_align));
            block->setOriAddr(begin() + phdr[num].p_offset);
            addLoadBlock(block);
        }
    }
    return true;
}

void Core::unload() {
}

const char* Core::getMachine() {
    return "arm64";
}

Core::~Core() {
}

} // namespace arm64
