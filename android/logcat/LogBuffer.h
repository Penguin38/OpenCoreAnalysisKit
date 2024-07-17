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

#ifndef ANDROID_LOGCAT_LOGBUFFER_H_
#define ANDROID_LOGCAT_LOGBUFFER_H_

#include "api/memory_ref.h"

struct LogBuffer_OffsetTable {
};

extern struct LogBuffer_OffsetTable __LogBuffer_offset__;

namespace android {

class LogBuffer : public api::MemoryRef {
public:
    LogBuffer() : api::MemoryRef() {}
    LogBuffer(uint64_t v) : api::MemoryRef(v) {}
    LogBuffer(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    LogBuffer(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    LogBuffer(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    LogBuffer(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
};

} // namespace android

#endif // ANDROID_LOGCAT_LOGBUFFER_H_
