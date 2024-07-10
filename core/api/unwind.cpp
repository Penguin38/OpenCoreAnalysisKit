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
#include "api/unwind.h"
#include "arm64/unwind.h"
#include "arm/unwind.h"
#include "x86/unwind.h"
#include "x86_64/unwind.h"
#include "riscv64/unwind.h"
#include "common/elf.h"

namespace api {

void UnwindStack::VisitFrame() {
    std::unique_ptr<NativeFrame> frame =
            std::make_unique<NativeFrame>(cur_frame_fp_,
                                          cur_frame_sp_,
                                          cur_frame_pc_);
    if (CoreApi::GetMachine() == EM_ARM) {
        /*
         * 31|30|29|28|27|26            8|7|6|5|4   0|
         *  N| Z| C| V| Q|       DNM     |I|F|T|M4~M0|
         */
        if (cur_state_ & 0b100000 || cur_frame_pc_ & 0x1) {
            frame->SetThumbMode();
        }
    }
    frame->Decode();
    native_frames_.push_back(std::move(frame));
    cur_num_++;
}

std::unique_ptr<UnwindStack> UnwindStack::MakeUnwindStack(ThreadApi* thread) {
    std::unique_ptr<UnwindStack> unwind;
    int machine = CoreApi::GetMachine();
    switch (machine) {
        case EM_386:
            unwind = std::make_unique<x86::UnwindStack>(thread);
            break;
        case EM_X86_64:
            unwind = std::make_unique<x86_64::UnwindStack>(thread);
            break;
        case EM_ARM:
            unwind = std::make_unique<arm::UnwindStack>(thread);
            break;
        case EM_AARCH64:
            unwind = std::make_unique<arm64::UnwindStack>(thread);
            break;
        case EM_RISCV:
            unwind = std::make_unique<riscv64::UnwindStack>(thread);
            break;
    }
    return std::move(unwind);
}

} // namespace api
