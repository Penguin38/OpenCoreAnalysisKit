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

#ifndef ANDROID_ART_RUNTIME_JAVA_FRAME_H_
#define ANDROID_ART_RUNTIME_JAVA_FRAME_H_

#include "runtime/art_method.h"
#include "runtime/oat_quick_method_header.h"
#include "runtime/interpreter/quick_frame.h"
#include "runtime/interpreter/shadow_frame.h"
#include "runtime/oat/stack_map.h"
#include <vector>

namespace art {

class JavaFrame {
public:
    JavaFrame(ArtMethod& m, QuickFrame& qf, ShadowFrame& sf)
        : method(m), quick_frame(qf), shadow_frame(sf) {}
    ArtMethod& GetMethod() { return method; }
    ShadowFrame& GetShadowFrame() { return shadow_frame; }
    QuickFrame& GetQuickFrame() { return quick_frame; }
    QuickFrame& GetPrevQuickFrame() { return prev_quick_frame; }
    OatQuickMethodHeader& GetMethodHeader() { return quick_frame.GetMethodHeader(); }
    uint64_t GetFramePc() { return quick_frame.Ptr() ? quick_frame.GetFramePc() : 0x0; }
    uint64_t GetDexPcPtr();
    std::map<uint32_t, CodeInfo::DexRegisterInfo>& GetVRegs() {
        if (shadow_frame.Ptr()) {
            return shadow_frame.GetVRegs();
        } else if (quick_frame.Ptr()) {
            return quick_frame.GetVRegs();
        }
        return empty_vregs;
    }
    void SetPrevQuickFrame(QuickFrame& qf) { prev_quick_frame = qf; }
private:
    ArtMethod method;
    ShadowFrame shadow_frame = 0x0;
    QuickFrame quick_frame = 0x0;
    QuickFrame prev_quick_frame = 0x0;
    std::map<uint32_t, CodeInfo::DexRegisterInfo> empty_vregs;
};

} // namespace

#endif // ANDROID_ART_RUNTIME_JAVA_FRAME_H_

