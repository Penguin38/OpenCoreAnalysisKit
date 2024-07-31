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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_FAKE_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_FAKE_SPACE_H_

#include "runtime/gc/space/space.h"
#include <functional>

namespace art {
namespace gc {
namespace space {

// [0x10000000, 0x12c00000) rw Fake space
class FakeSpace : public ContinuousMemMapAllocSpace {
public:
    static uint64_t FAKE_SPACE_PTR;
    static constexpr uint32_t FAKE_SPACE_MEMSIZE = 0x2000000;

    FakeSpace() : ContinuousMemMapAllocSpace() {}
    FakeSpace(uint64_t v) : ContinuousMemMapAllocSpace(v) {}
    FakeSpace(uint64_t v, LoadBlock* b) : ContinuousMemMapAllocSpace(v, b) {}
    FakeSpace(const ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(ref) {}
    FakeSpace(uint64_t v, ContinuousMemMapAllocSpace& ref) : ContinuousMemMapAllocSpace(v, ref) {}
    FakeSpace(uint64_t v, ContinuousMemMapAllocSpace* ref) : ContinuousMemMapAllocSpace(v, ref) {}

    static bool Create();
    SpaceType GetType() { return kSpaceTypeFakeSpace; }
    void Walk(std::function<bool (mirror::Object& object)> fn);
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_FAKE_SPACE_H_
