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

#include "logger/log.h"
#include "api/core.h"
#include "arm/unwind.h"
#include "common/ucontext.h"
#include "common/exception.h"
#include <string.h>

namespace arm {

uint32_t UnwindStack::GetUContext() {
    if (cur_uc_) return cur_uc_;
    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(GetThread());
    Register& regs = thread->GetRegs();

    uint8_t __padding[120];
    memset(&__padding, 0x0, sizeof(__padding));

    api::MemoryRef sp = regs.sp;
    sp.Prepare(false);
    LoadBlock* block = sp.Block();
    if (!block) return cur_uc_;

    try {
        api::MemoryRef uc = sp.Ptr() & CoreApi::GetVabitsMask();
        uc.checkCopyBlock(block);
        while(uc.Ptr() + sizeof(struct ucontext) < (block->vaddr() + block->size())) {
            struct ucontext* context = (struct ucontext*)uc.Real();
            if (block->virtualContains(context->uc_stack.ss_sp)
                    && context->uc_stack.ss_size <= block->memsz()
                    && context->uc_stack.ss_size >= CoreApi::GetPageSize()
                    && !(context->uc_stack.ss_size % CoreApi::GetPageSize())
                    // uc_link must 0
                    && context->uc_link == 0x0) {
                if (!memcmp(__padding, context->__padding, sizeof(__padding))) {
                    cur_uc_ = uc.Ptr();
                    break;
                }
            }
            uc.MovePtr(4);
        }
    } catch(InvalidAddressException& e) {
        // do nothing
    }
    return cur_uc_;
}

void UnwindStack::WalkStack() {
    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(GetThread());
    Register& regs = thread->GetRegs();
    Backtrace(regs);

    api::MemoryRef uc = GetUContext();
    if (uc.Ptr()) {
        uc_num_ = cur_num_;
        struct ucontext* context = (struct ucontext*)uc.Real();
        Register uc_regs;
        memcpy(&uc_regs, &context->uc_mcontext.arm_r0, sizeof(Register));
        Backtrace(uc_regs);
    }
}

void UnwindStack::Backtrace(Register& regs) {
    try {
        cur_frame_pc_ = regs.pc;
        cur_state_ = regs.cpsr;
        VisitFrame();
        cur_state_ = 0x0; // reset

        // cur_frame_pc_ = regs.lr;
        // VisitFrame();
    } catch(InvalidAddressException& e) {
        // do nothing
    }
}

void UnwindStack::DumpContextRegister(const char* prefix) {
    api::MemoryRef uc = GetUContext();
    if (uc.Ptr()) {
        struct ucontext* context = (struct ucontext*)uc.Real();
        Register uc_regs;
        memcpy(&uc_regs, &context->uc_mcontext.arm_r0, sizeof(Register));
        uc_regs.Dump(prefix);
    }
}

} // namespace arm
