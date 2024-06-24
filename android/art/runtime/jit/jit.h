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

#ifndef ANDROID_ART_RUNTIME_JIT_JIT_H_
#define ANDROID_ART_RUNTIME_JIT_JIT_H_

#include "api/memory_ref.h"
#include "runtime/jit/jit_code_cache.h"

struct Jit_OffsetTable {
    uint32_t code_cache_;
};

extern struct Jit_OffsetTable __Jit_offset__;

namespace art {
namespace jit {

class Jit : public api::MemoryRef {
public:
    Jit(uint64_t v) : api::MemoryRef(v) {}
    Jit(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Jit(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Jit(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init26();
    static void Init28();
    static void Init29();
    inline uint64_t code_cache() { return VALUEOF(Jit, code_cache_); }

    JitCodeCache& GetCodeCache();
private:
    // quick memoryref cache
    JitCodeCache code_cache_cache = 0x0;
};

} // namespace jit
} // namespace art

#endif // ANDROID_ART_RUNTIME_JIT_JIT_H_
