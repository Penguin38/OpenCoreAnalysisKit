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

#ifndef CORE_API_MEMORY_REF_H_
#define CORE_API_MEMORY_REF_H_

#include "api/core.h"
#include <stdint.h>
#include <sys/types.h>

/*
 *  api::MemoryRef ref = vaddr;
 *  va: ref.Ptr();
 *  ra: ref.Real();
 */

#define OFFSET(X, Y) (__##X##_offset__.Y)
#define MEMBER_SIZE(X, Y) (__##X##_size__.Y)
#define SIZE(X) (__##X##_size__.THIS)

namespace api {
class MemoryRef {
public:
    MemoryRef(uint64_t v) : vaddr(v), block(nullptr) {}
    MemoryRef(const MemoryRef& ref) : vaddr(ref.vaddr), block(ref.block) {}
    template<typename U> MemoryRef(U *v) { vaddr = reinterpret_cast<uint64_t>(v); }
    template<typename U> MemoryRef& operator=(U* other) { init(other); return *this; }
    template<typename U> void init(U* other) { vaddr = reinterpret_cast<uint64_t>(other); }

    bool operator==(uint64_t v) { return vaddr == v; }
    bool operator!=(uint64_t v) { return vaddr != v; }

    inline uint64_t Ptr() { return vaddr; }
    inline uint64_t Real() {
        if (!block) {
            block = CoreApi::FindLoadBlock(vaddr);
        }

        if (block) return block->begin() + (vaddr - block->vaddr());
        return 0x0;
    }
private:
    uint64_t vaddr;
    LoadBlock* block;
};
} // namespace api

#endif // CORE_API_MEMORY_REF_H_
