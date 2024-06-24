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

#ifndef ANDROID_ART_RUNTIME_MONITOR_POOL_H_
#define ANDROID_ART_RUNTIME_MONITOR_POOL_H_

#include "runtime/monitor.h"

struct MonitorPool_OffsetTable {
    uint32_t monitor_chunks_;
};

extern struct MonitorPool_OffsetTable __MonitorPool_offset__;

namespace art {

class MonitorPool : public api::MemoryRef {
public:
    MonitorPool(uint64_t v) : api::MemoryRef(v) {}
    MonitorPool(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    MonitorPool(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    MonitorPool(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static constexpr uint32_t kChunkSize = 4096;
    static constexpr uint32_t kMaxChunkLists = 8;
    static constexpr uint32_t kInitialChunkStorage = 256;
    static constexpr uint32_t kMaxListSize = kInitialChunkStorage << (kMaxChunkLists - 1);

    static void Init();
    inline uint64_t monitor_chunks() { return Ptr() + OFFSET(MonitorPool, monitor_chunks_); }

    static Monitor MonitorFromMonitorId(uint32_t mon_id);
    static MonitorPool& GetMonitorPool();
    uint32_t MonitorIdToOffset(uint32_t id);
    Monitor LookupMonitor(uint32_t mon_id);
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_MONITOR_POOL_H_
