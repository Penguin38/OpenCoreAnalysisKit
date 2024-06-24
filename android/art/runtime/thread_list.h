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

#ifndef ANDROID_ART_RUNTIME_THREAD_LIST_H_
#define ANDROID_ART_RUNTIME_THREAD_LIST_H_

#include "api/memory_ref.h"
#include "cxx/list.h"
#include "runtime/thread.h"
#include <list>
#include <memory>

struct ThreadList_OffsetTable {
    uint32_t list_;
};

struct ThreadList_SizeTable {
    uint32_t THIS;
};

extern struct ThreadList_OffsetTable __ThreadList_offset__;
extern struct ThreadList_SizeTable __ThreadList_size__;

namespace art {

class ThreadList : public api::MemoryRef {
public:
    static constexpr uint32_t kMaxThreadId = 0xFFFF;
    static constexpr uint32_t kInvalidThreadId = 0;
    static constexpr uint32_t kMainThreadId = 1;

    ThreadList(uint64_t v) : api::MemoryRef(v) {}
    ThreadList(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ThreadList(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ThreadList(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t list() { return Ptr() + OFFSET(ThreadList, list_); }

    cxx::list& GetListCache();
    std::list<std::unique_ptr<Thread>>& GetList();
    bool Contains(int tid);
    Thread* FindThreadByTid(int tid);
    Thread* FindThreadByThreadId(int id);

    void CleanCache() { list_second_cache.clear(); }
private:
    // quick memoryref cache
    cxx::list list_cache;

    // second cache
    std::list<std::unique_ptr<Thread>> list_second_cache;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_THREAD_LIST_H_
