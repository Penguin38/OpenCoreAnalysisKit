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

#ifndef ANDROID_FDTRACK_FDTRACK_H_
#define ANDROID_FDTRACK_FDTRACK_H_

#include "api/memory_ref.h"
#include <string>

struct FdEntry_OffsetTable {
    uint32_t backtrace;
};

struct FdEntry_SizeTable {
    uint32_t THIS;
};

extern struct FdEntry_OffsetTable __FdEntry_offset__;
extern struct FdEntry_SizeTable __FdEntry_size__;

namespace android {

class FdTrack {
public:
    static inline const char* FDTRACK64 = "/system/lib64/libfdtrack.so";
    static inline const char* FDTRACK32 = "/system/lib/libfdtrack.so";
    static inline const char* FD_TRACK_STACK_TRACES = "_ZL12stack_traces";
    static constexpr uint32_t kFdTableSize = 4096;
    static constexpr uint32_t kStackDepth = 32;

    static void Init();
    static const char* GetPath();
    static api::MemoryRef GetStackTraces();
    static api::MemoryRef AnalysisStackTraces();
};

class FdEntry : public api::MemoryRef {
public:
    FdEntry(uint64_t v) : api::MemoryRef(v) {}
    FdEntry(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    FdEntry(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    FdEntry(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    FdEntry(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t backtrace() { return Ptr() + OFFSET(FdEntry, backtrace); }
};

} // namespace android

#endif  // ANDROID_FDTRACK_FDTRACK_H_
