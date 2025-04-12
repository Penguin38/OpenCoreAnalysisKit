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

#include "arm64/thread_info.h"
#include "arm64/unwind.h"
#include "common/ucontext.h"
#include "common/prstatus.h"
#include "common/note_block.h"
#include "base/utils.h"
#include <stddef.h>
#include <string.h>
#include <memory>

namespace arm64 {

struct ThreadApi::RegsMap kMap[] = {
    {"x0", offsetof(Register, x0), sizeof(uint64_t)},
    {"x1", offsetof(Register, x1), sizeof(uint64_t)},
    {"x2", offsetof(Register, x2), sizeof(uint64_t)},
    {"x3", offsetof(Register, x3), sizeof(uint64_t)},
    {"x4", offsetof(Register, x4), sizeof(uint64_t)},
    {"x5", offsetof(Register, x5), sizeof(uint64_t)},
    {"x6", offsetof(Register, x6), sizeof(uint64_t)},
    {"x7", offsetof(Register, x7), sizeof(uint64_t)},
    {"x8", offsetof(Register, x8), sizeof(uint64_t)},
    {"x9", offsetof(Register, x9), sizeof(uint64_t)},

    {"x10", offsetof(Register, x10), sizeof(uint64_t)},
    {"x11", offsetof(Register, x11), sizeof(uint64_t)},
    {"x12", offsetof(Register, x12), sizeof(uint64_t)},
    {"x13", offsetof(Register, x13), sizeof(uint64_t)},
    {"x14", offsetof(Register, x14), sizeof(uint64_t)},
    {"x15", offsetof(Register, x15), sizeof(uint64_t)},
    {"x16", offsetof(Register, x16), sizeof(uint64_t)},
    {"x17", offsetof(Register, x17), sizeof(uint64_t)},
    {"x18", offsetof(Register, x18), sizeof(uint64_t)},
    {"x19", offsetof(Register, x19), sizeof(uint64_t)},

    {"x20", offsetof(Register, x20), sizeof(uint64_t)},
    {"x21", offsetof(Register, x21), sizeof(uint64_t)},
    {"x22", offsetof(Register, x22), sizeof(uint64_t)},
    {"x23", offsetof(Register, x23), sizeof(uint64_t)},
    {"x24", offsetof(Register, x24), sizeof(uint64_t)},
    {"x25", offsetof(Register, x25), sizeof(uint64_t)},
    {"x26", offsetof(Register, x26), sizeof(uint64_t)},
    {"x27", offsetof(Register, x27), sizeof(uint64_t)},
    {"x28", offsetof(Register, x28), sizeof(uint64_t)},
    {"x29", offsetof(Register, fp), sizeof(uint64_t)},

    {"fp", offsetof(Register, fp), sizeof(uint64_t)},
    {"lr", offsetof(Register, lr), sizeof(uint64_t)},
    {"sp", offsetof(Register, sp), sizeof(uint64_t)},
    {"pc", offsetof(Register, pc), sizeof(uint64_t)},

    {"cpsr", offsetof(Register, pstate), sizeof(uint64_t)},
    {"pst", offsetof(Register, pstate), sizeof(uint64_t)},
};

void ThreadInfo::RegisterSet(const char* command) {
    std::unique_ptr<char[], void(*)(void*)> newcommand(strdup(command), free);
    char *regs = strtok(newcommand.get(), "=");
    if (!regs) return;

    uint64_t value = 0x0;
    char *token = strtok(NULL, "=");
    if (token) value = Utils::atol(token);

    if (strcmp(regs, "ucontext")) {
        int count = sizeof(kMap)/sizeof(kMap[0]);
        for (int index = 0; index < count; ++index) {
            if (!strcmp(regs, kMap[index].regs)) {
                *(reinterpret_cast<uint64_t *>(&reg) + kMap[index].offset / 8) = value;
                block()->setOverlay(prs() + offsetof(Elf64_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
                return;
            }
        }
        LOGW("Invalid regs %s\n", regs);
    } else {
        api::MemoryRef uc = value;
        if (!uc.IsValid()) {
            UnwindStack stack(this);
            uc = stack.GetUContext();
        }
        if (uc.IsValid() && uc.Block()->virtualContains(
                uc.Ptr() + offsetof(struct ucontext, uc_mcontext)
                         + offsetof(struct mcontext, regs)
                         + sizeof(reg))) {
            struct ucontext* context = (struct ucontext*)uc.Real();
            memcpy(&reg, &context->uc_mcontext.regs, sizeof(reg));
            block()->setOverlay(prs() + offsetof(Elf64_prstatus, pr_reg), (void *)&reg, sizeof(reg));
        }
    }
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

} // namspace arm64
