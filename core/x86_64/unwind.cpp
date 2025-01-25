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
#include "x86_64/unwind.h"
#include "common/ucontext.h"
#include "common/exception.h"
#include <string.h>

namespace x86_64 {

uint64_t UnwindStack::GetUContext() {
    if (cur_uc_) return cur_uc_;
    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(GetThread());
    Register& regs = thread->GetRegs();

    uint64_t __reserved1[8];
    memset(&__reserved1, 0x0, sizeof(__reserved1));

    api::MemoryRef sp = regs.rsp;
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

                if (context->uc_mcontext.fpstate && !block->virtualContains(context->uc_mcontext.fpstate)) {
                    uc.MovePtr(8);
                    continue;
                }

                if (!memcmp(__reserved1, context->uc_mcontext.reserved1, sizeof(__reserved1))) {
                    api::MemoryRef uc_sp = context->uc_mcontext.sp;
                    if (uc_sp.IsValid()) {
                        cur_uc_ = uc.Ptr();
                        break;
                    }
                }
            }
            uc.MovePtr(8);
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
        uc_regs.r15 = context->uc_mcontext.r15;
        uc_regs.r14 = context->uc_mcontext.r14;
        uc_regs.r13 = context->uc_mcontext.r13;
        uc_regs.r12 = context->uc_mcontext.r12;
        uc_regs.rbp = context->uc_mcontext.bp;
        uc_regs.rbx = context->uc_mcontext.bx;
        uc_regs.r11 = context->uc_mcontext.r11;
        uc_regs.r10 = context->uc_mcontext.r10;
        uc_regs.r9 = context->uc_mcontext.r9;
        uc_regs.r8 = context->uc_mcontext.r8;
        uc_regs.rax = context->uc_mcontext.ax;
        uc_regs.rcx = context->uc_mcontext.cx;
        uc_regs.rdx = context->uc_mcontext.dx;
        uc_regs.rsi = context->uc_mcontext.si;
        uc_regs.rdi = context->uc_mcontext.di;
        uc_regs.rip = context->uc_mcontext.ip;
        uc_regs.cs = context->uc_mcontext.cs;
        uc_regs.flags = context->uc_mcontext.flags;
        uc_regs.rsp = context->uc_mcontext.sp;
        uc_regs.ss = context->uc_mcontext.ss;
        uc_regs.fs = context->uc_mcontext.fs;
        uc_regs.gs = context->uc_mcontext.gs;
        Backtrace(uc_regs);
    }
}

void UnwindStack::Backtrace(Register& regs) {
    try {
        cur_frame_pc_ = regs.rip;
        VisitFrame();
    } catch(InvalidAddressException& e) {
        // do nothing
    }
}

void UnwindStack::DumpContextRegister(const char* prefix) {
    api::MemoryRef uc = GetUContext();
    if (uc.Ptr()) {
        struct ucontext* context = (struct ucontext*)uc.Real();
        Register uc_regs;
        uc_regs.r15 = context->uc_mcontext.r15;
        uc_regs.r14 = context->uc_mcontext.r14;
        uc_regs.r13 = context->uc_mcontext.r13;
        uc_regs.r12 = context->uc_mcontext.r12;
        uc_regs.rbp = context->uc_mcontext.bp;
        uc_regs.rbx = context->uc_mcontext.bx;
        uc_regs.r11 = context->uc_mcontext.r11;
        uc_regs.r10 = context->uc_mcontext.r10;
        uc_regs.r9 = context->uc_mcontext.r9;
        uc_regs.r8 = context->uc_mcontext.r8;
        uc_regs.rax = context->uc_mcontext.ax;
        uc_regs.rcx = context->uc_mcontext.cx;
        uc_regs.rdx = context->uc_mcontext.dx;
        uc_regs.rsi = context->uc_mcontext.si;
        uc_regs.rdi = context->uc_mcontext.di;
        uc_regs.rip = context->uc_mcontext.ip;
        uc_regs.cs = context->uc_mcontext.cs;
        uc_regs.flags = context->uc_mcontext.flags;
        uc_regs.rsp = context->uc_mcontext.sp;
        uc_regs.ss = context->uc_mcontext.ss;
        uc_regs.fs = context->uc_mcontext.fs;
        uc_regs.gs = context->uc_mcontext.gs;
        uc_regs.Dump(prefix);
    }
}

} // namespace x86_64
