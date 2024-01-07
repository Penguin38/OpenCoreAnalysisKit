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

#ifndef CORE_LP32_CORE_H_
#define CORE_LP32_CORE_H_

#include "api/core.h"
#include <functional>

namespace lp32 {

class Auxv {
public:
    uint32_t type;
    uint32_t value;
};

class File {
public:
    uint32_t begin;
    uint32_t end;
    uint32_t offset;
};

class Core {
public:
    bool load32(CoreApi* api, std::function<void* (uint64_t, uint64_t)> callback);
};

} // namespace lp32

#endif // CORE_LP32_CORE_H_
