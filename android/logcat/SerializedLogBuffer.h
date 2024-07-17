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

#ifndef ANDROID_LOGCAT_SERIALIZED_LOGBUFFER_H_
#define ANDROID_LOGCAT_SERIALIZED_LOGBUFFER_H_

#include "api/memory_ref.h"

struct SerializedLogBuffer_OffsetTable {
    uint32_t vtbl;
    uint32_t reader_list_;
    uint32_t tags_;
    uint32_t stats_;
    uint32_t max_size_;
    uint32_t logs_;
};

struct SerializedLogBuffer_SizeTable {
    uint32_t THIS;
    uint32_t vtbl;
};

extern struct SerializedLogBuffer_OffsetTable __SerializedLogBuffer_offset__;
extern struct SerializedLogBuffer_SizeTable __SerializedLogBuffer_size__;

namespace android {

class SerializedLogBuffer : public api::MemoryRef {
public:
    SerializedLogBuffer() : api::MemoryRef() {}
    SerializedLogBuffer(uint64_t v) : api::MemoryRef(v) {}
    SerializedLogBuffer(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    SerializedLogBuffer(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    SerializedLogBuffer(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    SerializedLogBuffer(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init31();
    inline uint64_t vtbl() { return VALUEOF(SerializedLogBuffer, vtbl); }
    inline uint64_t reader_list() { return VALUEOF(SerializedLogBuffer, reader_list_); }
    inline uint64_t tags() { return VALUEOF(SerializedLogBuffer, tags_); }
    inline uint64_t stats() { return VALUEOF(SerializedLogBuffer, stats_); }
    inline uint64_t logs() { return Ptr() + OFFSET(SerializedLogBuffer, logs_); }
};

} // namespace android

#endif // ANDROID_LOGCAT_SERIALIZED_LOGBUFFER_H_
