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

#ifndef ANDROID_ART_RUNTIME_BASE_MUTEX_H_
#define ANDROID_ART_RUNTIME_BASE_MUTEX_H_

#include "api/memory_ref.h"

struct BaseMutex_OffsetTable {
    uint32_t vtbl;
    uint32_t vtbl_IsMutex;
    uint32_t vtbl_IsReaderWriterMutex;
    uint32_t vtbl_IsMutatorMutex;
    uint32_t name_;
};

extern struct BaseMutex_OffsetTable __BaseMutex_offset__;

struct Mutex_OffsetTable {
    uint32_t exclusive_owner_;
};

extern struct Mutex_OffsetTable __Mutex_offset__;

struct ReaderWriterMutex_OffsetTable {
    uint32_t state_;
    uint32_t exclusive_owner_;
};

extern struct ReaderWriterMutex_OffsetTable __ReaderWriterMutex_offset__;

namespace art {

class BaseMutex : public api::MemoryRef {
public:
    BaseMutex(uint64_t v) : api::MemoryRef(v) {}
    BaseMutex(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    BaseMutex(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    BaseMutex(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    struct LockTable {
        const char* name;
        const char* type;
    };

    static int kLockLevelCount;
    static void Init();
    inline uint64_t vtbl() { return VALUEOF(BaseMutex, vtbl); }
    inline uint64_t name() { return VALUEOF(BaseMutex, name_); }
    const char* GetName();
    virtual bool IsMutex();
    virtual bool IsReaderWriterMutex();
    virtual bool IsMutatorMutex();
    bool IsSpecialMutex(const char* type, uint32_t off);
    api::MemoryRef& GetVTBL();
private:
    // quick memoryref cache
    api::MemoryRef vtbl_cache = 0x0;
};

class Mutex : public BaseMutex {
public:
    Mutex(uint64_t v) : BaseMutex(v) {}
    Mutex(const BaseMutex& ref) : BaseMutex(ref) {}
    Mutex(uint64_t v, BaseMutex& ref) : BaseMutex(v, ref) {}
    Mutex(uint64_t v, BaseMutex* ref) : BaseMutex(v, ref) {}

    static void Init();
    inline uint32_t exclusive_owner() { return value32Of(OFFSET(Mutex, exclusive_owner_)); }
    bool IsMutex() { return true; }
    uint32_t GetExclusiveOwnerTid();
};

class ReaderWriterMutex : public BaseMutex {
public:
    ReaderWriterMutex(uint64_t v) : BaseMutex(v) {}
    ReaderWriterMutex(const BaseMutex& ref) : BaseMutex(ref) {}
    ReaderWriterMutex(uint64_t v, BaseMutex& ref) : BaseMutex(v, ref) {}
    ReaderWriterMutex(uint64_t v, BaseMutex* ref) : BaseMutex(v, ref) {}

    static void Init();
    inline int32_t state() { return value32Of(OFFSET(ReaderWriterMutex, state_)); }
    inline uint32_t exclusive_owner() { return value32Of(OFFSET(ReaderWriterMutex, exclusive_owner_)); }
    bool IsReaderWriterMutex() { return true; }
    uint32_t GetExclusiveOwnerTid();
};

class MutatorMutex : public ReaderWriterMutex {
public:
    MutatorMutex(uint64_t v) : ReaderWriterMutex(v) {}
    MutatorMutex(const ReaderWriterMutex& ref) : ReaderWriterMutex(ref) {}
    MutatorMutex(uint64_t v, ReaderWriterMutex& ref) : ReaderWriterMutex(v, ref) {}
    MutatorMutex(uint64_t v, ReaderWriterMutex* ref) : ReaderWriterMutex(v, ref) {}

    bool IsMutatorMutexMutex() { return true; }
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_BASE_MUTEX_H_
