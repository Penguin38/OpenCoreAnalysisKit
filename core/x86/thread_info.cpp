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

#include "x86/thread_info.h"
#include "x86/unwind.h"
#include "common/ucontext.h"
#include "common/prstatus.h"
#include "common/note_block.h"
#include "base/utils.h"
#include <stddef.h>
#include <string.h>
#include <memory>

namespace x86 {

struct ThreadApi::RegsMap kMap[] = {
    {"ebx", offsetof(Register, ebx), sizeof(uint32_t)},
    {"ecx", offsetof(Register, ecx), sizeof(uint32_t)},
    {"edx", offsetof(Register, edx), sizeof(uint32_t)},
    {"esi", offsetof(Register, esi), sizeof(uint32_t)},
    {"edi", offsetof(Register, edi), sizeof(uint32_t)},
    {"ebp", offsetof(Register, ebp), sizeof(uint32_t)},
    {"eax", offsetof(Register, eax), sizeof(uint32_t)},
    {"eip", offsetof(Register, eip), sizeof(uint32_t)},
    {"esp", offsetof(Register, esp), sizeof(uint32_t)},
    {"eflags", offsetof(Register, eflags), sizeof(uint32_t)},
    {"orig_eax", offsetof(Register, orig_eax), sizeof(uint32_t)},

    {"ds", offsetof(Register, ds), sizeof(uint16_t)},
    {"es", offsetof(Register, es), sizeof(uint16_t)},
    {"fs", offsetof(Register, fs), sizeof(uint16_t)},
    {"gs", offsetof(Register, gs), sizeof(uint16_t)},
    {"cs", offsetof(Register, cs), sizeof(uint16_t)},
    {"ss", offsetof(Register, ss), sizeof(uint16_t)},
};

void ThreadInfo::RegisterSet(const char* command) {
    std::unique_ptr<char> newcommand(strdup(command));
    char *regs = strtok(newcommand.get(), "=");
    if (!regs) return;

    uint32_t value = 0x0;
    char *token = strtok(NULL, "=");
    if (token) value = atoi(token);

    if (strcmp(regs, "ucontext")) {
        int count = sizeof(kMap)/sizeof(kMap[0]);
        for (int index = 0; index < count; ++index) {
            if (!strcmp(regs, kMap[index].regs)) {
                if (kMap[index].size == sizeof(uint32_t)) {
                    *(reinterpret_cast<uint32_t *>(&reg) + kMap[index].offset / 4) = value;
                    block()->setOverlay(prs() + offsetof(Elf32_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
                } else {
                    value = value & 0xFFFF;
                    *(reinterpret_cast<uint16_t *>(&reg) + kMap[index].offset / 2) = value;
                    block()->setOverlay(prs() + offsetof(Elf32_prstatus, pr_reg) + kMap[index].offset, (void *)&value, kMap[index].size);
                }
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
                         + offsetof(struct mcontext, oldmask))) {
            struct ucontext* context = (struct ucontext*)uc.Real();
            Register uc_regs;
            uc_regs.ebx = context->uc_mcontext.bx;
            uc_regs.ecx = context->uc_mcontext.cx;
            uc_regs.edx = context->uc_mcontext.dx;
            uc_regs.esi = context->uc_mcontext.si;
            uc_regs.edi = context->uc_mcontext.di;
            uc_regs.ebp = context->uc_mcontext.bp;
            uc_regs.eax = context->uc_mcontext.ax;
            uc_regs.ds = context->uc_mcontext.ds;
            uc_regs.es = context->uc_mcontext.es;
            uc_regs.fs = context->uc_mcontext.fs;
            uc_regs.gs = context->uc_mcontext.gs;
            uc_regs.orig_eax = context->uc_mcontext.ax;
            uc_regs.eip = context->uc_mcontext.ip;
            uc_regs.eflags = context->uc_mcontext.flags;
            uc_regs.esp = context->uc_mcontext.sp;
            uc_regs.ss = context->uc_mcontext.ss;
            memcpy(&reg, &uc_regs, sizeof(reg));
            block()->setOverlay(prs() + offsetof(Elf32_prstatus, pr_reg), (void *)&reg, sizeof(reg));
        }
    }
}

uint64_t ThreadInfo::RegisterGet(const char* regs) {
    int count = sizeof(kMap)/sizeof(kMap[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(regs, kMap[index].regs)) {
            if (kMap[index].size == sizeof(uint32_t)) {
                return *(reinterpret_cast<uint32_t *>(&reg) + kMap[index].offset / 4);
            } else {
                return *(reinterpret_cast<uint16_t *>(&reg) + kMap[index].offset / 2);
            }
        }
    }
    return 0;
}

} // namspace x86
