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

#include "logger/log.h"
#include "api/core.h"
#include "common/exception.h"
#include "backtrace/callstack.h"
#include <stdint.h>
#include <sys/types.h>
#include <iostream>

/*
 *  api::MemoryRef ref = vaddr;
 *  va: ref.Ptr();
 *  ra: ref.Real();
 */

#define OFFSET(X, Y) (__##X##_offset__.Y)
#define MEMBER_SIZE(X, Y) (__##X##_size__.Y)
#define SIZEOF(X) (__##X##_size__.THIS)
#define VALUEOF(X, Y) (*reinterpret_cast<uint64_t *>(Real() + OFFSET(X, Y)) & PointMask())

namespace api {
class MemoryRef {
public:
    MemoryRef() {}
    MemoryRef(uint64_t v) : vaddr(v), block(nullptr) {}
    MemoryRef(const MemoryRef& ref) : vaddr(ref.vaddr), block(ref.block) {}
    MemoryRef(uint64_t v, LoadBlock* b) : vaddr(v) { checkCopyBlock(b); }
    MemoryRef(uint64_t v, MemoryRef& ref) : vaddr(v) { copyRef(ref); }
    MemoryRef(uint64_t v, MemoryRef* ref) : vaddr(v) { copyRef(ref); }

    template<typename U> MemoryRef(U *v) { init(v); }
    template<typename U> MemoryRef(U *v, LoadBlock* b) { init(v); checkCopyBlock(b); }
    template<typename U> MemoryRef(U *v, MemoryRef& ref) { init(v); copyRef(ref); }
    template<typename U> MemoryRef(U *v, MemoryRef* ref) { init(v); copyRef(ref); }

    template<typename U> void init(U* other) { vaddr = reinterpret_cast<uint64_t>(other); block = nullptr; }
    inline void checkCopyBlock(LoadBlock* b) { if (b && b->virtualContains(vaddr)) block = b; }
    inline void copyRef(MemoryRef& ref) { checkCopyBlock(ref.block); }
    inline void copyRef(MemoryRef* ref) { checkCopyBlock(ref->block); }

    bool operator==(uint64_t v) { return vaddr == v; }
    bool operator!=(uint64_t v) { return vaddr != v; }

    inline uint64_t Ptr() { return vaddr; }
    inline uint64_t Real() {
        if (!vaddr)
            throw InvalidAddressException(vaddr);

        Prepare(true);
        return block->begin() + ((vaddr & block->VabitsMask()) - block->vaddr());
    }
    inline uint64_t PointMask() { return block->PointMask(); }
    inline bool IsReady() { return block != nullptr; }
    inline void Prepare(bool check) {
        if (!block) {
#if 0 // cloctime debug
            static int count = 0;
            count++;
            LOGI("Prepare %d\n", count);
            CallStack stack;
            stack.update(2);
            stack.dump(1);
#endif
            block = CoreApi::FindLoadBlock(vaddr, check);
        }
    }
    inline void MovePtr(int64_t length) {
        if (!vaddr)
            throw InvalidAddressException(vaddr);

        Prepare(true);
        if (block && block->virtualContains(vaddr + length))
            vaddr = vaddr + length;
    }
private:
    uint64_t vaddr;
    LoadBlock* block;
};
} // namespace api

#endif // CORE_API_MEMORY_REF_H_
