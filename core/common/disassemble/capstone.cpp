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

#include "api/core.h"
#include "common/elf.h"
#include "common/disassemble/capstone.h"
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#if defined(__CAPSTONE__)
#include "capstone/capstone.h"
#endif // __CAPSTONE__

namespace capstone {

// CS_MODE_THUMB = 1 << 4, ///< ARM's Thumb mode, including Thumb-2
int Disassember::ARM_MODE = 1 << 4;

void Disassember::SetArmMode(const char* mode) {
#if defined(__CAPSTONE__)
    if (!strcmp(mode, "thumb")) {
        ARM_MODE = CS_MODE_THUMB;
    } else {
        ARM_MODE = CS_MODE_ARM;
    }
#endif // __CAPSTONE__
}

void Disassember::Dump(const char* prefix, api::MemoryRef& begin, uint32_t size, uint64_t address, Option& opt) {
    try {
        Dump(prefix, (uint8_t *)begin.Real(), size, address, opt);
    } catch(InvalidAddressException e) {
        // do nothing
    }
}

void Disassember::Dump(const char* prefix, uint8_t* begin, uint32_t size, uint64_t address, Option& opt) {
#if defined(__CAPSTONE__)
    csh handle;
    cs_insn *insn;
    uint32_t count;

    cs_arch arch;
    cs_mode mode;

    if (opt.force) {
        arch = static_cast<cs_arch>(opt.arch);
        mode = static_cast<cs_mode>(opt.mode);
    } else {
        if (!CoreApi::IsReady())
            return;

        int machine = CoreApi::GetMachine();
        switch (machine) {
            case EM_386:
                arch = CS_ARCH_X86;
                mode = CS_MODE_32;
                break;
            case EM_X86_64:
                arch = CS_ARCH_X86;
                mode = CS_MODE_64;
                break;
            case EM_ARM:
                arch = CS_ARCH_ARM;
                mode = static_cast<cs_mode>(ARM_MODE);
                break;
            case EM_AARCH64:
                arch = CS_ARCH_AARCH64;
                mode = CS_MODE_LITTLE_ENDIAN;
                break;
            case EM_RISCV:
                arch = CS_ARCH_RISCV;
                mode = CS_MODE_RISCV64;
                break;
        }
    }

    if (cs_open(arch, mode, &handle) != CS_ERR_OK)
        return;

    count = cs_disasm(handle, begin, size, address, 0, &insn);
    if (count > 0) {
        uint32_t num = 0;
        for (uint32_t i = 0; i < count; ++i) {
            if (insn[i].address < opt.start)
                continue;

            if (num >= opt.num)
                break;

            std::string machine_code;
            std::ostringstream ss;
            for (uint32_t k = 0; k < insn[i].size; ++k) {
                std::ostringstream ss;
                ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(insn[i].bytes[k]);
                machine_code.append(ss.str());
            }

            if (arch == CS_ARCH_X86) {
                LOGI("%s0x%lx: %24s | %s %s\n",
                        prefix, insn[i].address, machine_code.c_str(), insn[i].mnemonic, insn[i].op_str);
            } else {
                LOGI("%s0x%lx: %8s | %s %s\n",
                        prefix, insn[i].address, machine_code.c_str(), insn[i].mnemonic, insn[i].op_str);
            }
            num++;
        }
    }
    cs_free(insn, count);
#endif  // __CAPSTONE__
}

} // namespace capstone
