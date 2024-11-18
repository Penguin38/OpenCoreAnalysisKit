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

#include "riscv64/thread_info.h"
#include "common/prstatus.h"
#include "common/note_block.h"
#include "base/utils.h"
#include <stddef.h>
#include <string.h>
#include <memory>

namespace riscv64 {

struct ThreadApi::RegsMap kMap[] = {
    {"pc", offsetof(Register, pc), sizeof(uint64_t)},
    {"ra", offsetof(Register, ra), sizeof(uint64_t)},
    {"sp", offsetof(Register, sp), sizeof(uint64_t)},
    {"gp", offsetof(Register, gp), sizeof(uint64_t)},
    {"tp", offsetof(Register, tp), sizeof(uint64_t)},
    {"t0", offsetof(Register, t0), sizeof(uint64_t)},
    {"t1", offsetof(Register, t1), sizeof(uint64_t)},
    {"t2", offsetof(Register, t2), sizeof(uint64_t)},
    {"s0", offsetof(Register, s0), sizeof(uint64_t)},
    {"s1", offsetof(Register, s1), sizeof(uint64_t)},

    {"a0", offsetof(Register, a0), sizeof(uint64_t)},
    {"a1", offsetof(Register, a1), sizeof(uint64_t)},
    {"a2", offsetof(Register, a2), sizeof(uint64_t)},
    {"a3", offsetof(Register, a3), sizeof(uint64_t)},
    {"a4", offsetof(Register, a4), sizeof(uint64_t)},
    {"a5", offsetof(Register, a5), sizeof(uint64_t)},
    {"a6", offsetof(Register, a6), sizeof(uint64_t)},
    {"a7", offsetof(Register, a7), sizeof(uint64_t)},

    {"s2", offsetof(Register, s2), sizeof(uint64_t)},
    {"s3", offsetof(Register, s3), sizeof(uint64_t)},
    {"s4", offsetof(Register, s4), sizeof(uint64_t)},
    {"s5", offsetof(Register, s5), sizeof(uint64_t)},
    {"s6", offsetof(Register, s6), sizeof(uint64_t)},
    {"s7", offsetof(Register, s7), sizeof(uint64_t)},
    {"s8", offsetof(Register, s8), sizeof(uint64_t)},
    {"s9", offsetof(Register, s9), sizeof(uint64_t)},
    {"s10", offsetof(Register, s10), sizeof(uint64_t)},
    {"s11", offsetof(Register, s11), sizeof(uint64_t)},

    {"t3", offsetof(Register, t3), sizeof(uint64_t)},
    {"t4", offsetof(Register, t4), sizeof(uint64_t)},
    {"t5", offsetof(Register, t5), sizeof(uint64_t)},
    {"t6", offsetof(Register, t6), sizeof(uint64_t)},
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
            *(reinterpret_cast<uint64_t *>(&reg) + kMap[index].offset / 8) = value;
            block()->setOverlay(prs() + offsetof(Elf64_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
            return;
        }
    }
    LOGW("Invalid regs %s\n", regs);
}

uint64_t ThreadInfo::RegisterGet(const char* regs) {
    int count = sizeof(kMap)/sizeof(kMap[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(regs, kMap[index].regs)) {
            return *(reinterpret_cast<uint64_t *>(&reg) + kMap[index].offset / 8);
        }
    }
    return 0;
}

} // namspace riscv64
