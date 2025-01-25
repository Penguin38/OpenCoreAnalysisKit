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
#include "x86/unwind.h"
#include "common/ucontext.h"
#include "common/exception.h"
#include <string.h>

namespace x86 {

uint32_t UnwindStack::GetUContext() {
    if (cur_uc_) return cur_uc_;
    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(GetThread());
    Register& regs = thread->GetRegs();

    api::MemoryRef sp = regs.esp;
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
                    uc.MovePtr(4);
                    continue;
                }

                api::MemoryRef uc_sp = context->uc_mcontext.sp;
                if (uc_sp.IsValid()) {
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
        Backtrace(uc_regs);
    }
}

void UnwindStack::Backtrace(Register& regs) {
    try {
        cur_frame_pc_ = regs.eip;
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
        uc_regs.Dump(prefix);
    }
}

} // namespace x86
