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
#include "command/env.h"
#include "command/fake/core/arm64/fake_core.h"
#include "base/memory_map.h"
#include <linux/elf.h>

namespace arm64 {

int FakeCore::execute(const char* output) {
    // For test FakeCore
    Elf64_Ehdr ehdr;
    snprintf((char *)ehdr.e_ident, 5, ELFMAG);
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;

    ehdr.e_type = ET_CORE;
    ehdr.e_machine = EM_AARCH64;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = 0x0;
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_shoff = 0x0;
    ehdr.e_flags = 0x0;
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = 0;
    ehdr.e_shentsize = 0x0;
    ehdr.e_shnum = 0x0;
    ehdr.e_shstrndx = 0x0;

    std::unique_ptr<MemoryMap> map(MemoryMap::MmapMem((uint64_t)&ehdr, sizeof(Elf64_Ehdr)));
    CoreApi::Load(map, false, Env::Init);
    std::unique_ptr<::FakeCore> impl = FakeCore::Make(CoreApi::Bits());
    return impl ? impl->execute(output) : 0;
}

} // namespace arm64
