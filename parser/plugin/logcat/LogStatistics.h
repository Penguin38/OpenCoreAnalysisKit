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

#ifndef PARSER_PLUGIN_LOGCAT_LOG_STATISTICS_H_
#define PARSER_PLUGIN_LOGCAT_LOG_STATISTICS_H_

#include "api/memory_ref.h"

struct LogStatistics_OffsetTable {
    uint32_t mElements;
};

extern struct LogStatistics_OffsetTable __LogStatistics_offset__;

namespace android {

class LogStatistics : public api::MemoryRef {
public:
    LogStatistics() : api::MemoryRef() {}
    LogStatistics(uint64_t v) : api::MemoryRef(v) {}
    LogStatistics(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    LogStatistics(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    LogStatistics(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    LogStatistics(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t mElements() { return Ptr() + OFFSET(LogStatistics, mElements); }
};

} // namespace android

#endif // PARSER_PLUGIN_LOGCAT_LOG_STATISTICS_H_
