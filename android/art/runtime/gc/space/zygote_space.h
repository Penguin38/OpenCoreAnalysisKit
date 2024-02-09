/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_ZYGOTE_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_ZYGOTE_SPACE_H_

#include "runtime/gc/space/space.h"
#include <functional>

struct ZygoteSpace_OffsetTable {
};

struct ZygoteSpace_SizeTable {
    uint32_t THIS;
};

extern struct ZygoteSpace_OffsetTable __ZygoteSpace_offset__;
extern struct ZygoteSpace_SizeTable __ZygoteSpace_size__;

namespace art {
namespace gc {
namespace space {

class ZygoteSpace : public ContinuousMemMapAllocSpace {
public:
    ZygoteSpace() : ContinuousMemMapAllocSpace() {}
    ZygoteSpace(uint64_t v) : ContinuousMemMapAllocSpace(v) {}
    ZygoteSpace(uint64_t v, LoadBlock* b) : ContinuousMemMapAllocSpace(v, b) {}
    ZygoteSpace(const ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(ref) {}
    ZygoteSpace(uint64_t v, ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(v, ref) {}
    ZygoteSpace(uint64_t v, ContinuousMemMapAllocSpace* ref) : ContinuousMemMapAllocSpace(v, ref) {}
    template<typename U> ZygoteSpace(U *v) : ContinuousMemMapAllocSpace(v) {}
    template<typename U> ZygoteSpace(U *v, ContinuousMemMapAllocSpace* ref) : ContinuousMemMapAllocSpace(v, ref) {}

    static void Init();

    SpaceType GetType() { return kSpaceTypeZygoteSpace; }
    void Walk(std::function<bool (mirror::Object& object)> fn);
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_ZYGOTE_SPACE_H_
