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

#ifndef ANDROID_ART_RUNTIME_MANAGED_STACK_H_
#define ANDROID_ART_RUNTIME_MANAGED_STACK_H_

#include "api/memory_ref.h"
#include "runtime/interpreter/shadow_frame.h"

struct ManagedStack_OffsetTable {
    uint32_t tagged_top_quick_frame_;
    uint32_t link_;
    uint32_t top_shadow_frame_;
};

struct ManagedStack_SizeTable {
    uint32_t THIS;
};

extern struct ManagedStack_OffsetTable __ManagedStack_offset__;
extern struct ManagedStack_SizeTable __ManagedStack_size__;

namespace art {

class ManagedStack : public api::MemoryRef {
public:
    ManagedStack(uint64_t v) : api::MemoryRef(v) {}
    ManagedStack(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ManagedStack(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ManagedStack(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> ManagedStack(U *v) : api::MemoryRef(v) {}
    template<typename U> ManagedStack(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    class TaggedTopQuickFrame : public api::MemoryRef {
    public:
        TaggedTopQuickFrame(uint64_t v) : api::MemoryRef(v) {}
        TaggedTopQuickFrame(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
        TaggedTopQuickFrame(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
        TaggedTopQuickFrame(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
        template<typename U> TaggedTopQuickFrame(U *v) : api::MemoryRef(v) {}
        template<typename U> TaggedTopQuickFrame(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

        inline uint64_t tagged_sp() { return valueOf(); }
        inline uint64_t GetSp() { return tagged_sp() & (~static_cast<uint64_t>(3u) & PointMask()); }
        inline bool GetGenericJniTag() { return (tagged_sp() & 1u) != 0u; }
        inline bool GetJitJniTag() { return (tagged_sp() & 2u) != 0u; }
    };

    static void Init();
    inline uint64_t tagged_top_quick_frame() { return Ptr() + OFFSET(ManagedStack, tagged_top_quick_frame_); }
    inline uint64_t link() { return VALUEOF(ManagedStack, link_); }
    inline uint64_t top_shadow_frame() { return VALUEOF(ManagedStack, top_shadow_frame_); }

    TaggedTopQuickFrame& GetTaggedTopQuickFrame();
    ShadowFrame& GetTopShadowFrame();
    inline uint64_t GetTopQuickFrame() { return GetTaggedTopQuickFrame().GetSp(); }
    inline bool GetTopQuickFrameGenericJniTag() { return GetTaggedTopQuickFrame().GetGenericJniTag(); }
    inline bool GetTopQuickFrameJitJniTag() { return GetTaggedTopQuickFrame().GetJitJniTag(); }
private:
    TaggedTopQuickFrame tagged_top_quick_frame_cache = 0x0;
    ShadowFrame top_shadow_frame_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_MANAGED_STACK_H_
