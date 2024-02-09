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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_IMAGE_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_IMAGE_SPACE_H_

#include "runtime/gc/space/space.h"
#include <functional>

struct ImageSpace_OffsetTable {
};

struct ImageSpace_SizeTable {
    uint32_t THIS;
};

extern struct ImageSpace_OffsetTable __ImageSpace_offset__;
extern struct ImageSpace_SizeTable __ImageSpace_size__;

namespace art {
namespace gc {
namespace space {

class ImageSpace : public MemMapSpace {
public:
    ImageSpace() : MemMapSpace() {}
    ImageSpace(uint64_t v) : MemMapSpace(v) {}
    ImageSpace(uint64_t v, LoadBlock* b) : MemMapSpace(v, b) {}
    ImageSpace(const MemMapSpace& ref) : MemMapSpace(ref) {}
    ImageSpace(uint64_t v, MemMapSpace& ref) : MemMapSpace(v, ref) {}
    ImageSpace(uint64_t v, MemMapSpace* ref) : MemMapSpace(v, ref) {}
    template<typename U> ImageSpace(U *v) : MemMapSpace(v) {}
    template<typename U> ImageSpace(U *v, MemMapSpace* ref) : MemMapSpace(v, ref) {}

    static void Init();

    SpaceType GetType() { return kSpaceTypeImageSpace; }
    void Walk(std::function<bool (mirror::Object& object)> fn);
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_IMAGE_SPACE_H_
