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

#ifndef ANDROID_LOGCAT_SERAALIZED_DATA_H_
#define ANDROID_LOGCAT_SERAALIZED_DATA_H_

#include "api/memory_ref.h"

struct SerializedData_OffsetTable {
    uint32_t data_;
    uint32_t size_;
};

extern struct SerializedData_OffsetTable __SerializedData_offset__;

namespace android {

class SerializedData : public api::MemoryRef {
public:
    static constexpr int FILTER_PID = 1 << 0;
    static constexpr int FILTER_UID = 1 << 1;
    static constexpr int FILTER_TID = 1 << 2;

    SerializedData(uint64_t v) : api::MemoryRef(v) {}
    SerializedData(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    SerializedData(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    SerializedData(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    SerializedData(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t data() { return VALUEOF(SerializedData, data_); }
    inline uint64_t size() { return VALUEOF(SerializedData, size_); }

    void DecodeDump(int filter, int id);

    class SerializedElement {
    public:
        uint32_t uid;
        uint32_t pid;
        uint32_t tid;
        uint32_t tv_sec;
        uint32_t tv_nsec;
        uint32_t msg_len;
        uint8_t log_id;
        uint32_t event_id;
        std::string tag;
        std::string msg;
    };
};

} // namespace android

#endif // ANDROID_LOGCAT_SERAALIZED_DATA_H_
