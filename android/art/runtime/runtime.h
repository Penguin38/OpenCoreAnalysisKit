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

#ifndef ANDROID_ART_RUNTIME_RUNTIME_H_
#define ANDROID_ART_RUNTIME_RUNTIME_H_

#include "logger/log.h"
#include "api/memory_ref.h"
#include "runtime/gc/heap.h"
#include "runtime/thread_list.h"

struct Runtime_OffsetTable {
    uint32_t callee_save_methods_;
    uint32_t heap_;
    uint32_t thread_list_;
};

struct Runtime_SizeTable {
    uint32_t THIS;
};

extern struct Runtime_OffsetTable __Runtime_offset__;
extern struct Runtime_SizeTable __Runtime_size__;

namespace art {

class Runtime : public api::MemoryRef {
public:
    Runtime() : api::MemoryRef() {}
    Runtime(uint64_t v) : api::MemoryRef(v) {}
    Runtime(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    Runtime(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Runtime(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Runtime(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> Runtime(U *v) : api::MemoryRef(v) {}
    template<typename U> Runtime(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(Runtime& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Runtime& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    static void Init31();
    static void Init33();
    static void Init34();
    inline uint64_t heap() { return VALUEOF(Runtime, heap_); }
    inline uint64_t thread_list() { return VALUEOF(Runtime, thread_list_); }

    static Runtime& Current();
    gc::Heap& GetHeap();
    ThreadList& GetThreadList();

    void CleanCache() {
        if (heap_cache.Ptr()) heap_cache.CleanCache();
        if (thread_list_cache.Ptr()) thread_list_cache.CleanCache();
    }
private:
    static Runtime AnalysisInstance();
    // quick memoryref cache
    gc::Heap heap_cache = 0x0;
    ThreadList thread_list_cache = 0x0;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_RUNTIME_H_
