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

#include "runtime/gc/space/fake_space.h"
#include "runtime/runtime_globals.h"
#include "api/core.h"
#include "cxx/string.h"

namespace art {
namespace gc {
namespace space {

bool FakeSpace::Create() {
    if (!CoreApi::NewLoadBlock(FAKE_SPACE_PTR, FAKE_SPACE_MEMSIZE))
        return false;

    // cxx::string Fake space
    uint64_t name_buf[3] = {0x707320656B614614ULL,
                            0x0000000000656361ULL,
                            0x0000000000000000ULL};
    CoreApi::Write(FAKE_SPACE_PTR + OFFSET(Space, name_), name_buf, SIZEOF(cxx_string));

    CoreApi::Write(FAKE_SPACE_PTR + OFFSET(ContinuousSpace, begin_), FAKE_SPACE_BEGIN);
    CoreApi::Write(FAKE_SPACE_PTR + OFFSET(ContinuousSpace, end_), FAKE_SPACE_END);
    return true;
}

void FakeSpace::Walk(std::function<bool (mirror::Object& object)> visitor) {
    uint64_t pos = Begin();
    uint64_t top = End();
    mirror::Object object_cache = pos;
    object_cache.Prepare(false);

    while (pos < top) {
        mirror::Object object(pos, object_cache);
        if (object.IsValid()) {
            visitor(object);
            pos = GetNextObject(object);
        } else {
            pos += kObjectAlignment;
        }
    }
}

} // namespace space
} // namespace gc
} // namespace art
