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

#ifndef ANDROID_ART_RUNTIME_INTERPRETER_QUICK_FRAME_H_
#define ANDROID_ART_RUNTIME_INTERPRETER_QUICK_FRAME_H_

#include "api/memory_ref.h"
#include "runtime/art_method.h"
#include <vector>

namespace art {

class QuickFrame : public api::MemoryRef {
public:
    QuickFrame(uint64_t v) : api::MemoryRef(v) {}
    QuickFrame(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    QuickFrame(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    QuickFrame(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    void init(OatQuickMethodHeader& mh, uint64_t pc) {
        method_header = mh;
        frame_pc = pc;
    }

    inline ArtMethod& GetMethod() {
        if (!method.Ptr()) {
            method = valueOf();
        }
        return method;
    }
    inline OatQuickMethodHeader& GetMethodHeader() { return method_header; }
    uint64_t GetFramePc() { return frame_pc; }
    uint64_t GetDexPcPtr();
    std::vector<uint32_t>& GetVRegs();
    std::vector<uint32_t>& GetVRegsCache() { return vregs_cache; }
private:
    ArtMethod method = 0x0;
    OatQuickMethodHeader method_header = 0x0;
    uint64_t frame_pc;
    std::vector<uint32_t> vregs_cache;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_INTERPRETER_QUICK_FRAME_H_
