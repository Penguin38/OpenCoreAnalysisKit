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

#include "runtime/thread_list.h"
#include "common/exception.h"

struct ThreadList_OffsetTable __ThreadList_offset__;
struct ThreadList_SizeTable __ThreadList_size__;

namespace art {

void ThreadList::Init() {
    __ThreadList_offset__ = {
        .list_ = 8192,
    };
}

cxx::list& ThreadList::GetListCache() {
    if (!list_cache.Ptr()) {
        list_cache = list();
        list_cache.copyRef(this);
    }
    return list_cache;
}

std::list<std::unique_ptr<Thread>>& ThreadList::GetList() {
    if (!list_second_cache.size()) {
        try {
            for (const auto& value : GetListCache()) {
                api::MemoryRef ref = value;
                std::unique_ptr<Thread> thread = std::make_unique<Thread>(ref.valueOf());
                if (thread->IsValid())
                    list_second_cache.push_back(std::move(thread));
            }
        } catch (InvalidAddressException e) {}
    }
    return list_second_cache;
}

bool ThreadList::Contains(int tid) {
    for (const auto& thread : GetList()) {
        if (thread->GetTid() == tid)
            return true;
    }
    return false;
}

Thread* ThreadList::FindThreadByTid(int tid) {
    for (const auto& thread : GetList()) {
        if (thread->GetTid() == tid)
            return thread.get();
    }
    return nullptr;
}

Thread* ThreadList::FindThreadByThreadId(int id) {
    for (const auto& thread : GetList()) {
        if (thread->GetThreadId() == id)
            return thread.get();
    }
    return nullptr;
}

} //namespace art
