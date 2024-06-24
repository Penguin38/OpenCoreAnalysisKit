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

#include "api/core.h"
#include "runtime/runtime.h"
#include "runtime/monitor_pool.h"

struct MonitorPool_OffsetTable __MonitorPool_offset__;

namespace art {

void MonitorPool::Init() {
    if (CoreApi::Bits() == 64) {
        __MonitorPool_offset__ = {
            .monitor_chunks_ = 0,
        };
    } else {
        __MonitorPool_offset__ = {
            .monitor_chunks_ = 0,
        };
    }
}

Monitor MonitorPool::MonitorFromMonitorId(uint32_t mon_id) {
    return GetMonitorPool().LookupMonitor(mon_id);
}

MonitorPool& MonitorPool::GetMonitorPool() {
    return Runtime::Current().GetMonitorPool();
}

uint32_t MonitorPool::MonitorIdToOffset(uint32_t id) {
    return id << 3;
}

Monitor MonitorPool::LookupMonitor(uint32_t mon_id) {
    uint32_t offset = MonitorIdToOffset(mon_id);
    uint32_t index = offset / kChunkSize;
    uint32_t top_index = index / kMaxListSize;
    uint32_t list_index = index % kMaxListSize;
    uint32_t offset_in_chunk = offset % kChunkSize;
    api::MemoryRef monitor_chunks_ = monitor_chunks();
    uint32_t point_size = CoreApi::GetPointSize();
    api::MemoryRef top_chuck = monitor_chunks_.valueOf(top_index * point_size);
    uint64_t base = top_chuck.valueOf(list_index * point_size);
    return (base + offset_in_chunk);
}

} // namespace art
