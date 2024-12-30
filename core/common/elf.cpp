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

#include "common/elf.h"
#include "api/core.h"
#include <linux/elf.h>

bool ElfHeader::CheckLibrary(const char* file) {
    if (memcmp(ident, ELFMAG, 4)) {
        LOGE("Invalid ELF file (%s)\n", file);
        return false;
    }

    if (type != ET_DYN) {
        LOGE("Invalid shared object file (%s)\n", file);
        return false;
    }

    if (machine != CoreApi::GetMachine()) {
        LOGE("Invalid match machine(%d) (%s)\n", machine, file);
        return false;
    }

    return true;
}

std::string ElfHeader::ToMachineName(uint16_t machine) {
    switch(machine) {
        case EM_AARCH64: return "arm64";
        case EM_RISCV: return "riscv64";
        case EM_X86_64: return "x86_64";
        case EM_ARM: return "arm";
        case EM_386: return "x86";
    }
    return std::to_string(machine);
}
