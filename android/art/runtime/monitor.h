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

#ifndef ANDROID_ART_RUNTIME_MONITOR_H_
#define ANDROID_ART_RUNTIME_MONITOR_H_

#include "runtime/thread_list.h"
#include "runtime/mirror/object.h"

struct Monitor_OffsetTable {
    uint32_t owner_;
    uint32_t obj_;
};

extern struct Monitor_OffsetTable __Monitor_offset__;

namespace art {

class Monitor : public api::MemoryRef {
public:
    Monitor(uint64_t v) : api::MemoryRef(v) {}
    Monitor(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Monitor(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Monitor(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init26();
    static void Init30();
    inline uint64_t owner() { return VALUEOF(Monitor, owner_); }
    inline uint64_t obj() { return VALUEOF(Monitor, obj_); }

    static ThreadState FetchState(Thread* thread, mirror::Object* monitor_object, uint32_t* lock_owner_tid);
    static uint32_t GetLockOwnerThreadId(mirror::Object& obj);
    inline mirror::Object GetObject() { return obj(); }
    inline Thread GetOwner() { return owner(); }
    inline uint32_t GetOwnerThreadId() { return GetOwner().Ptr() ? GetOwner().GetThreadId() : ThreadList::kInvalidThreadId; }
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_MONITOR_H_
