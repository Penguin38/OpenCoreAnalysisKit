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
#include "arm64/unwind.h"
#include "common/elf.h"
#include "common/ucontext.h"
#include "common/exception.h"
#include <string.h>

namespace arm64 {

uint64_t UnwindStack::GetUContext() {
    if (cur_uc_) return cur_uc_;
    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(GetThread());
    Register& regs = thread->GetRegs();

    uint8_t __reserved[120 + 8];
    memset(&__reserved, 0x0, sizeof(__reserved));

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
                if (!memcmp(__reserved, context->__reserved, sizeof(__reserved))) {
                    cur_uc_ = uc.Ptr();
                    break;
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
    FpBacktrace(regs);

    api::MemoryRef uc = GetUContext();
    if (uc.Ptr()) {
        uc_num_ = cur_num_;
        struct ucontext* context = (struct ucontext*)uc.Real();
        Register uc_regs;
        memcpy(&uc_regs, &context->uc_mcontext.regs, sizeof(Register));
        FpBacktrace(uc_regs);
    }
}

void UnwindStack::FpBacktrace(Register& regs) {
    try {
        cur_frame_fp_ = regs.fp;
        cur_frame_pc_ = regs.pc;

        // FP CALLTRACE
        VisitFrame();
        LoadBlock* vdso = CoreApi::FindLoadBlock(CoreApi::FindAuxv(AT_SYSINFO_EHDR), false);
        cur_frame_pc_ = regs.lr;
        if (!vdso || !vdso->virtualContains(cur_frame_pc_))
            cur_frame_pc_ -= 0x4;

        api::MemoryRef fp = cur_frame_fp_;
        fp.Prepare(false);
        LoadBlock* block = fp.Block();
        if (!block || (regs.lr != fp.value64Of(8))) {
            VisitFrame();

            // skip lr frame
            std::unique_ptr<NativeFrame>& lr_frame = native_frames_[native_frames_.size() - 1];
            std::unique_ptr<NativeFrame>& pc_frame = native_frames_[native_frames_.size() - 2];
            std::string& lr_symbol = lr_frame->GetMethodSymbol();
            if ((lr_symbol.length() && lr_symbol == pc_frame->GetMethodSymbol())
                    || lr_frame->GetFramePc() == pc_frame->GetFramePc())
                native_frames_.pop_back();
        }

    } catch(InvalidAddressException& e) {
        // do nothing
    }

    OnlyFpBackStack(cur_frame_fp_);
}

void UnwindStack::OnlyFpBackStack(uint64_t curfp) {
    try {
        LoadBlock* vdso = CoreApi::FindLoadBlock(CoreApi::FindAuxv(AT_SYSINFO_EHDR), false);

        api::MemoryRef fp = curfp;
        fp.Prepare(false);
        LoadBlock* block = fp.Block();
        if (!block) return;

        while (1) {
            cur_frame_pc_ = fp.value64Of(8);
            if (!vdso || !vdso->virtualContains(cur_frame_pc_))
                cur_frame_pc_ -= 0x4;
            cur_frame_fp_ = fp.value64Of();

            if (cur_frame_fp_ <= fp.Ptr())
                break;

            fp = cur_frame_fp_;
            fp.checkCopyBlock(block);
            if (block != fp.Block())
                break;

            VisitFrame();
        }
    } catch(InvalidAddressException& e) {
        // do nothing
    }
}

void UnwindStack::DumpContextRegister(const char* prefix) {
    api::MemoryRef uc = GetUContext();
    if (uc.Ptr()) {
        struct ucontext* context = (struct ucontext*)uc.Real();
        Register uc_regs;
        memcpy(&uc_regs, &context->uc_mcontext.regs, sizeof(Register));
        uc_regs.Dump(prefix);
    }
}

} // namespace arm64
