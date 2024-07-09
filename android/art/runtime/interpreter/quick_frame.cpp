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

#include "logger/log.h"
#include "api/core.h"
#include "android.h"
#include "common/bit.h"
#include "common/elf.h"
#include "runtime/nterp_helpers.h"
#include "runtime/interpreter/quick_frame.h"
#include "runtime/runtime.h"
#include "runtime/handle_scope.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/entrypoints/quick/callee_save_frame.h"

namespace art {

void QuickFrame::SetFramePc(uint64_t pc) {
    if (CoreApi::GetMachine() == EM_ARM) {
        frame_pc = pc & (CoreApi::GetPointMask() - 1);
    } else {
        frame_pc = pc;
    }
}

uint64_t QuickFrame::GetDexPcPtr() {
    if (GetMethod().IsNative()) {
        return 0x0;
    } else if (method_header.Ptr()) {
        if (method_header.IsOptimized()) {
            uint32_t native_pc = static_cast<uint32_t>(frame_pc - method_header.GetCodeStart());
            uint32_t dex_pc = method_header.NativePc2DexPc(native_pc);
            ArtMethod method = GetMethod();
            art::dex::CodeItem item = method.GetCodeItem();
            return item.Ptr() ? item.Ptr() + item.code_offset_ + 0x2 * dex_pc : 0x0;
        } else {
            return NterpGetFrameDexPcPtr(*this);
        }
    }
    return 0x0;
}

std::map<uint32_t, DexRegisterInfo>& QuickFrame::GetVRegs() {
    if (vregs_cache.size()) {
        return vregs_cache;
    }

    if (GetMethod().IsNative()) {
        // do nothing
    } else if (method_header.Ptr()) {
        if (method_header.IsOptimized()) {
            uint32_t native_pc = static_cast<uint32_t>(frame_pc - method_header.GetCodeStart());
            method_header.NativePc2VRegs(native_pc, vregs_cache);
        } else {
            NterpGetFrameVRegs(*this);
        }
    }
    return vregs_cache;
}

static uint32_t GetNumberOfReferenceArgsWithoutReceiver(ArtMethod& method) {
    uint32_t shorty_len;
    const char* shorty = method.GetShorty(&shorty_len);
    uint32_t refs = 0;
    for (uint32_t i = 1; i < shorty_len ; ++i) {
        if (shorty[i] == 'L') {
            refs++;
        }
    }
    return refs;
}

QuickMethodFrameInfo QuickFrame::GetFrameInfo() {
    if (GetMethodHeader().Ptr()) {
        if(Android::Sdk() >= Android::R) {
            if (GetMethodHeader().IsOptimized()) {
                return GetMethodHeader().GetFrameInfo();
            } else {
                return NterpFrameInfo(*this);
            }
        } else {
            return GetMethodHeader().GetFrameInfo();
        }
    }

    ArtMethod method = GetMethod();
    Runtime& runtime = Runtime::Current();

    if (method.IsAbstract()) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
    }

    if (method.IsRuntimeMethod()) {
        return runtime.GetRuntimeMethodFrameInfo(method);
    }

    if (method.IsProxyMethod()) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
    }

    if (Android::Sdk() >= Android::R) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
    } else {
        uint32_t handle_refs = GetNumberOfReferenceArgsWithoutReceiver(method) + 1;
        uint32_t scope_size = HandleScope::SizeOf(handle_refs);
        QuickMethodFrameInfo callee_info = RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
        uint32_t frame_size = RoundUp(callee_info.FrameSizeInBytes() + scope_size, kStackAlignment);
        return QuickMethodFrameInfo(frame_size, callee_info.CoreSpillMask(), callee_info.FpSpillMask());
    }
}

} //namespace art
