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

#ifndef ANDROID_ART_RUNTIME_THREAD_H_
#define ANDROID_ART_RUNTIME_THREAD_H_

#include "api/memory_ref.h"
#include "runtime/thread_state.h"
#include "runtime/mirror/object.h"
#include "runtime/base/mutex.h"

struct Thread_OffsetTable {
    uint32_t tls32_;
    uint32_t tlsPtr_;
    uint32_t wait_monitor_;
};

struct Thread_SizeTable {
    uint32_t THIS;
};

extern struct Thread_OffsetTable __Thread_offset__;
extern struct Thread_SizeTable __Thread_size__;

struct Thread_tls_32bit_sized_values_OffsetTable {
    uint32_t state_and_flags;
    uint32_t suspend_count;
    uint32_t thin_lock_thread_id;
    uint32_t tid;
};

struct Thread_tls_32bit_sized_values_SizeTable {
    uint32_t THIS;
};

extern struct Thread_tls_32bit_sized_values_OffsetTable __Thread_tls_32bit_sized_values_offset__;
extern struct Thread_tls_32bit_sized_values_SizeTable __Thread_tls_32bit_sized_values_size__;

struct Thread_tls_ptr_sized_values_OffsetTable {
    uint32_t stack_end;
    uint32_t managed_stack;
    uint32_t self;
    uint32_t opeer;
    uint32_t stack_begin;
    uint32_t stack_size;
    uint32_t monitor_enter_object;
    uint32_t name;
    uint32_t pthread_self;
    uint32_t held_mutexes;
};

struct Thread_tls_ptr_sized_values_SizeTable {
    uint32_t THIS;
};

extern struct Thread_tls_ptr_sized_values_OffsetTable __Thread_tls_ptr_sized_values_offset__;
extern struct Thread_tls_ptr_sized_values_SizeTable __Thread_tls_ptr_sized_values_size__;

namespace art {

class Thread : public api::MemoryRef {
public:
    static constexpr uint32_t kMaxThreadId = 0xFFFF;
    static constexpr uint32_t kInvalidThreadId = 0;
    static constexpr uint32_t kMainThreadId = 1;

    Thread(uint64_t v) : api::MemoryRef(v) {}
    Thread(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Thread(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Thread(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init26();
    static void Init28();
    static void Init29();
    static void Init30();
    static void Init31();
    static void Init33();
    static void Init34();
    static void Init35();
    inline uint64_t tls32() { return Ptr() + OFFSET(Thread, tls32_); }
    inline uint64_t tlsPtr() { return Ptr() + OFFSET(Thread, tlsPtr_); }
    inline uint64_t wait_monitor() { return VALUEOF(Thread, wait_monitor_); }

    class tls_32bit_sized_values : public api::MemoryRef {
    public:
        tls_32bit_sized_values(uint64_t v) : api::MemoryRef(v) {}
        tls_32bit_sized_values(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        tls_32bit_sized_values(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        tls_32bit_sized_values(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init26();
        static void Init31();
        inline uint32_t state_and_flags() { return value32Of(OFFSET(Thread_tls_32bit_sized_values, state_and_flags)); }
        inline uint32_t suspend_count() { return value32Of(OFFSET(Thread_tls_32bit_sized_values, suspend_count)); }
        inline uint32_t thin_lock_thread_id() { return value32Of(OFFSET(Thread_tls_32bit_sized_values, thin_lock_thread_id)); }
        inline uint32_t tid() { return value32Of(OFFSET(Thread_tls_32bit_sized_values, tid)); }
    };

    class tls_ptr_sized_values : public api::MemoryRef {
    public:
        tls_ptr_sized_values(uint64_t v) : api::MemoryRef(v) {}
        tls_ptr_sized_values(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        tls_ptr_sized_values(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        tls_ptr_sized_values(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        static void Init26();
        static void Init30();
        static void Init33();
        static void Init34();
        inline uint64_t stack_end() { return VALUEOF(Thread_tls_ptr_sized_values, stack_end); }
        inline uint64_t managed_stack() { return Ptr() + OFFSET(Thread_tls_ptr_sized_values, managed_stack); }
        inline uint64_t self() { return VALUEOF(Thread_tls_ptr_sized_values, self); }
        inline uint64_t opeer() { return VALUEOF(Thread_tls_ptr_sized_values, opeer); }
        inline uint64_t stack_begin() { return VALUEOF(Thread_tls_ptr_sized_values, stack_begin); }
        inline uint64_t stack_size() { return VALUEOF(Thread_tls_ptr_sized_values, stack_size); }
        inline uint64_t monitor_enter_object() { return VALUEOF(Thread_tls_ptr_sized_values, monitor_enter_object); }
        inline uint64_t name() { return VALUEOF(Thread_tls_ptr_sized_values, name); }
        inline uint64_t pthread_self() { return VALUEOF(Thread_tls_ptr_sized_values, pthread_self); }
        inline uint64_t held_mutexes() { return Ptr() + OFFSET(Thread_tls_ptr_sized_values, held_mutexes); }
    };

    tls_32bit_sized_values& GetTls32();
    tls_ptr_sized_values& GetTlsPtr();

    uint16_t GetFlags();
    ThreadState GetState();
    const char* GetStateDescriptor();
    uint32_t GetThreadId();
    uint32_t GetTid();
    const char* GetName();
    void DumpState();
    uint64_t GetWaitMonitor();
    mirror::Object GetMonitorEnterObject();
    BaseMutex GetHeldMutex(uint32_t level);
private:
    // quick memoryref cache
    tls_32bit_sized_values tls32_cache = 0x0;
    tls_ptr_sized_values tlsPtr_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_THREAD_H_
