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

#include "arm/thread_info.h"
#include "common/prstatus.h"
#include "common/note_block.h"
#include "base/utils.h"
#include <stddef.h>
#include <string.h>
#include <memory>

namespace arm {

struct ThreadApi::RegsMap kMap[] = {
    {"r0", offsetof(Register, r0), sizeof(uint32_t)},
    {"r1", offsetof(Register, r1), sizeof(uint32_t)},
    {"r2", offsetof(Register, r2), sizeof(uint32_t)},
    {"r3", offsetof(Register, r3), sizeof(uint32_t)},
    {"r4", offsetof(Register, r4), sizeof(uint32_t)},
    {"r5", offsetof(Register, r5), sizeof(uint32_t)},
    {"r6", offsetof(Register, r6), sizeof(uint32_t)},
    {"r7", offsetof(Register, r7), sizeof(uint32_t)},
    {"r8", offsetof(Register, r8), sizeof(uint32_t)},
    {"r9", offsetof(Register, r9), sizeof(uint32_t)},

    {"r10", offsetof(Register, r10), sizeof(uint32_t)},
    {"r11", offsetof(Register, r11), sizeof(uint32_t)},
    {"r12", offsetof(Register, r12), sizeof(uint32_t)},
    {"sp", offsetof(Register, sp), sizeof(uint32_t)},
    {"lr", offsetof(Register, lr), sizeof(uint32_t)},
    {"pc", offsetof(Register, pc), sizeof(uint32_t)},
    {"cpsr", offsetof(Register, cpsr), sizeof(uint32_t)},
};

void ThreadInfo::RegisterSet(const char* command) {
    std::unique_ptr<char> newcommand(strdup(command));
    char *regs = strtok(newcommand.get(), "=");
    if (!regs) return;

    char *token = strtok(NULL, "=");
    uint32_t value = atoi(token);

    int count = sizeof(kMap)/sizeof(kMap[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(regs, kMap[index].regs)) {
            *(reinterpret_cast<uint32_t *>(&reg) + kMap[index].offset / 4) = value;
            block()->setOverlay(prs() + offsetof(Elf32_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
            return;
        }
    }
    LOGW("Invalid regs %s\n", regs);
}

uint64_t ThreadInfo::RegisterGet(const char* regs) {
    int count = sizeof(kMap)/sizeof(kMap[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(regs, kMap[index].regs)) {
            return *(reinterpret_cast<uint32_t *>(&reg) + kMap[index].offset / 4);
        }
    }
    return 0;
}

} // namspace arm
