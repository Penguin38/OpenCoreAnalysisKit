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

#include "runtime/hprof/hprof.h"
#include "runtime/runtime.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/space.h"
#include "cxx/vector.h"

namespace art {
namespace hprof {

void Hprof::DumpHprof(const char* output, bool visible) {
    LOGI("hprof: heap dump %s starting...\n", output);
    Runtime& runtime = Runtime::Current();
    gc::Heap& heap = runtime.GetHeap();
    LOGI("runtime 0x%lx\n", runtime.Ptr());
    LOGI("heap 0x%lx\n", heap.Ptr());
    LOGI("continuous_spaces_ %lx\n", heap.GetContinuousSpaces().__begin());
    LOGI("discontinuous_spaces_ %lx\n", heap.GetDiscontinuousSpaces().__begin());

    for (const auto& value : heap.GetContinuousSpaces()) {
        api::MemoryRef ref = value;
        gc::space::Space space = ref.valueOf();
        LOGI("%s\n", space.GetName());
    }

    for (const auto& value : heap.GetDiscontinuousSpaces()) {
        api::MemoryRef ref = value;
        gc::space::Space space = ref.valueOf();
        LOGI("%s\n", space.GetName());
    }
}

} // namespace hprof
} // namespace art
