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

#ifndef ANDROID_UNWIND_STACK_UNWINDER_H_
#define ANDROID_UNWIND_STACK_UNWINDER_H_

#include "api/memory_ref.h"
#include <string>

struct FrameData_OffsetTable {
    uint32_t pc;
    uint32_t function_name;
    uint32_t function_offset;
};

struct FrameData_SizeTable {
    uint32_t THIS;
};

extern struct FrameData_OffsetTable __FrameData_offset__;
extern struct FrameData_SizeTable __FrameData_size__;

namespace android {

class UnwindStack {
public:
    static void Init();

    class FrameData : public api::MemoryRef {
    public:
        FrameData(uint64_t v) : api::MemoryRef(v) {}
        FrameData(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
        FrameData(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        FrameData(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        FrameData(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init();
        inline uint64_t pc() { return value64Of(OFFSET(FrameData, pc)); }
        inline uint64_t function_name() { return VALUEOF(FrameData, function_name); }
        inline uint64_t function_offset() { return value64Of(OFFSET(FrameData, function_offset)); }
        std::string GetMethod();
    };
};

} // namespace android

#endif  // ANDROID_UNWIND_STACK_UNWINDER_H_
