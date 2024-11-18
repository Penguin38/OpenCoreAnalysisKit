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

#include "x86_64/thread_info.h"
#include "common/prstatus.h"
#include "common/note_block.h"
#include "base/utils.h"
#include <stddef.h>
#include <string.h>
#include <memory>

namespace x86_64 {

struct ThreadApi::RegsMap kMap[] = {
    {"r15", offsetof(Register, r15), sizeof(uint64_t)},
    {"r14", offsetof(Register, r14), sizeof(uint64_t)},
    {"r13", offsetof(Register, r13), sizeof(uint64_t)},
    {"r12", offsetof(Register, r12), sizeof(uint64_t)},

    {"rbp", offsetof(Register, rbp), sizeof(uint64_t)},
    {"rbx", offsetof(Register, rbx), sizeof(uint64_t)},

    {"r11", offsetof(Register, r11), sizeof(uint64_t)},
    {"r10", offsetof(Register, r10), sizeof(uint64_t)},
    {"r9", offsetof(Register, r9), sizeof(uint64_t)},
    {"r8", offsetof(Register, r8), sizeof(uint64_t)},

    {"rax", offsetof(Register, rax), sizeof(uint64_t)},
    {"rcx", offsetof(Register, rcx), sizeof(uint64_t)},
    {"rdx", offsetof(Register, rdx), sizeof(uint64_t)},
    {"rsi", offsetof(Register, rsi), sizeof(uint64_t)},
    {"rdi", offsetof(Register, rdi), sizeof(uint64_t)},
    {"orig_rax", offsetof(Register, orig_rax), sizeof(uint64_t)},
    {"rip", offsetof(Register, rip), sizeof(uint64_t)},
    {"rsp", offsetof(Register, rsp), sizeof(uint64_t)},
    {"flags", offsetof(Register, flags), sizeof(uint64_t)},

    {"ds", offsetof(Register, ds), sizeof(uint32_t)},
    {"es", offsetof(Register, es), sizeof(uint32_t)},
    {"fs", offsetof(Register, fs), sizeof(uint32_t)},
    {"gs", offsetof(Register, gs), sizeof(uint32_t)},
    {"cs", offsetof(Register, cs), sizeof(uint32_t)},
    {"ss", offsetof(Register, ss), sizeof(uint32_t)},
};

void ThreadInfo::RegisterSet(const char* command) {
    std::unique_ptr<char> newcommand(strdup(command));
    char *regs = strtok(newcommand.get(), "=");
    if (!regs) return;

    char *token = strtok(NULL, "=");
    uint64_t value = Utils::atol(token);

    int count = sizeof(kMap)/sizeof(kMap[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(regs, kMap[index].regs)) {
            if (kMap[index].size == sizeof(uint64_t)) {
                *(reinterpret_cast<uint64_t *>(&reg) + kMap[index].offset / 8) = value;
                block()->setOverlay(prs() + offsetof(Elf64_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
            } else {
                value = value & 0xFFFFFFFF;
                *(reinterpret_cast<uint32_t *>(&reg) + kMap[index].offset / 4) = value;
                block()->setOverlay(prs() + offsetof(Elf64_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
            }
            return;
        }
    }
    LOGW("Invalid regs %s\n", regs);
}

uint64_t ThreadInfo::RegisterGet(const char* regs) {
    int count = sizeof(kMap)/sizeof(kMap[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(regs, kMap[index].regs)) {
            if (kMap[index].size == sizeof(uint64_t)) {
                return *(reinterpret_cast<uint64_t *>(&reg) + kMap[index].offset / 8);
            } else {
                return *(reinterpret_cast<uint32_t *>(&reg) + kMap[index].offset / 4);
            }
        }
    }
    return 0;
}

} // namspace x86_64
