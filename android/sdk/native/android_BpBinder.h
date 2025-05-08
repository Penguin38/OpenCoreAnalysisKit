/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef ANDROID_SDK_NATIVE_ANDROID_BPBINDER_H_
#define ANDROID_SDK_NATIVE_ANDROID_BPBINDER_H_

#include "api/memory_ref.h"

struct BpBinder_OffsetTable {
    uint32_t mAlive;
    uint32_t mObjects;
    uint32_t mDescriptorCache;
    uint32_t mTrackedUid;
};

extern struct BpBinder_OffsetTable __BpBinder_offset__;

namespace android {

class BpBinder : public api::MemoryRef {
public:
    BpBinder(uint64_t v) : api::MemoryRef(v) {}
    BpBinder(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    BpBinder(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    BpBinder(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    BpBinder(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static void Init30();
    static void Init31();
    static void Init33();
    static void Init34();
    static void Init35();
    static void Init36();

    inline int32_t mAlive() { return value32Of(OFFSET(BpBinder, mAlive)); }
    inline uint64_t mObjects() { return Ptr() + OFFSET(BpBinder, mObjects); }
    inline uint64_t mDescriptorCache() { return VALUEOF(BpBinder, mDescriptorCache); }
    inline int32_t mTrackedUid() { return value32Of(OFFSET(BpBinder, mTrackedUid)); }
};

} // android

#endif // ANDROID_SDK_NATIVE_ANDROID_BPBINDER_H_
