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

#include "runtime/gc/space/space.h"
#include "cxx/string.h"

struct Space_OffsetTable __Space_offset__;
struct Space_SizeTable __Space_size__;

namespace art {
namespace gc {
namespace space {

void Space::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __Space_offset__ = {
            .name_ = 8,
        };
    } else {
        __Space_offset__ = {
            .name_ = 4,
        };
    }
}

const char* Space::GetName() {
    cxx::string name_(name(), this);
    return name_.c_str();
}

} // namespace space
} // namespace gc
} // namespace art
