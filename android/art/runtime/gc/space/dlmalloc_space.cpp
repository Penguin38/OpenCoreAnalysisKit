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

#include "logger/log.h"
#include "runtime/gc/space/dlmalloc_space.h"
#include "runtime/runtime_globals.h"
#include "common/bit.h"

namespace art {
namespace gc {
namespace space {

void DlMallocSpace::Init() {
    // do nothing
}

void DlMallocSpace::Walk(std::function<bool (mirror::Object& object)> visitor, bool check) {
    // do nothing
}

uint64_t DlMallocSpace::GetNextObject(mirror::Object& object) {
    const uint64_t position = object.Ptr() + object.SizeOf();
    return RoundUp(position, kObjectAlignment * 2);
}

} // namespace space
} // namespace gc
} // namespace art
