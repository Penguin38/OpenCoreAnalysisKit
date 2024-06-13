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
#include "runtime/nterp_helpers.h"
#include "runtime/interpreter/quick_frame.h"
#include "runtime/runtime.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/entrypoints/quick/callee_save_frame.h"

namespace art {

uint64_t QuickFrame::GetDexPcPtr() {
    if (GetMethod().IsNative()) {
        return 0x0;
    } else if (method_header.Ptr()) {
        if (method_header.IsOptimized()) {
            return method_header.NativePc2DexPc(frame_pc);
        } else {
            return NterpGetFrameDexPcPtr(*this);
        }
    }
    return 0x0;
}

std::vector<uint32_t>& QuickFrame::GetVRegs() {
    if (vregs_cache.size()) {
        return vregs_cache;
    }

    if (GetMethod().IsNative()) {
        // do nothing
    } else if (method_header.Ptr()) {
        if (method_header.IsOptimized()) {
            // do nothing
        } else {
            NterpGetFrameVRegs(*this);
        }
    }
    return vregs_cache;
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

    return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
}

} //namespace art
