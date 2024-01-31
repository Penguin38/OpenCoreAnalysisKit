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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_SPACE_H_

#include "logger/log.h"
#include "api/memory_ref.h"

struct Space_OffsetTable {
    uint32_t name_;
};

struct Space_SizeTable {
    uint32_t THIS;
};

extern struct Space_OffsetTable __Space_offset__;
extern struct Space_SizeTable __Space_size__;

namespace art {
namespace gc {
namespace space {

class Space : public api::MemoryRef {
public:
    Space() : api::MemoryRef() {}
    Space(uint64_t v) : api::MemoryRef(v) {}
    Space(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    Space(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Space(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Space(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> Space(U *v) : api::MemoryRef(v) {}
    template<typename U> Space(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(Space& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Space& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t name() { return Ptr() + OFFSET(Space, name_); }

    const char* GetName();
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_SPACE_H_
