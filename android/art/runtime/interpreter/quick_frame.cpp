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
#include "runtime/arch/x86_64/registers_x86_64.h"
#include "runtime/arch/x86/registers_x86.h"
#include "runtime/arch/arm64/registers_arm64.h"
#include "runtime/arch/arm/registers_arm.h"
#include "runtime/arch/riscv64/registers_riscv64.h"

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

uint64_t QuickFrame::ReturnPc2FramePc(uint64_t rpc) {
    // ART OAT CODE
    if (rpc) {
        int machine = CoreApi::GetMachine();
        switch (machine) {
            // 1850ff | call dword ptr [eax + 0x18]
            case EM_386: return rpc - 0x3;
            // 2057ff | call qword ptr [rdi + 0x20]
            case EM_X86_64: return rpc - 0x3;
            // 47f0 | blx lr
            case EM_ARM: return rpc - 0x2;
            // d63f03c0 | blr x30
            case EM_AARCH64: return rpc - 0x4;
            case EM_RISCV: return rpc - 0x4;
        }
    }
    return rpc;
}

std::string QuickFrame::RegisterDesc(int idx, bool compat) {
    std::string sb;
    int machine = CoreApi::GetMachine();
    switch (machine) {
        case EM_386: {
            sb = x86::RegisterName(static_cast<x86::Register>(idx));
            if (sb.length()) return sb;
        } break;
        case EM_X86_64: {
            sb = x86_64::RegisterName(static_cast<x86_64::Register>(idx));
            if (sb.length()) return sb;
        } break;
        case EM_ARM: {
            sb = arm::RegisterName(static_cast<arm::Register>(idx));
            if (sb.length()) return sb;
        } break;
        case EM_AARCH64: {
            if (!compat) {
                sb = arm64::XRegisterName(static_cast<arm64::XRegister>(idx));
            } else {
                sb = arm64::WRegisterName(static_cast<arm64::WRegister>(idx));
            }
            if (sb.length()) return sb;
        } break;
        case EM_RISCV: {
            sb = riscv64::XRegisterName(static_cast<riscv64::XRegister>(idx));
            if (sb.length()) return sb;
        } break;
    }
    sb.append("r");
    sb.append(std::to_string(idx));
    return sb;
}

} //namespace art
