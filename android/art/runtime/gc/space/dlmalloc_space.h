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

#ifndef ANDROID_ART_RUNTIME_GC_SPACE_DLMALLOC_SPACE_H_
#define ANDROID_ART_RUNTIME_GC_SPACE_DLMALLOC_SPACE_H_

#include "runtime/gc/space/malloc_space.h"
#include <functional>

namespace art {
namespace gc {
namespace space {

class DlMallocSpace : public MallocSpace {
public:
    DlMallocSpace(uint64_t v) : MallocSpace(v) {}
    DlMallocSpace(uint64_t v, LoadBlock* b) : MallocSpace(v, b) {}
    DlMallocSpace(const MallocSpace& ref) : MallocSpace(ref) {}
    DlMallocSpace(uint64_t v, MallocSpace& ref) : MallocSpace(v, ref) {}
    DlMallocSpace(uint64_t v, MallocSpace* ref) : MallocSpace(v, ref) {}

    static void Init();
    bool IsRosAllocSpace() { return false; }
    bool IsDlMallocSpace() { return true; }
    uint64_t GetNextObject(mirror::Object& object);
    void Walk(std::function<bool (mirror::Object& object)> fn, bool check);
};

} // namespace space
} // namespace gc
} // namespace art

#endif // ANDROID_ART_RUNTIME_GC_SPACE_DLMALLOC_SPACE_H_
