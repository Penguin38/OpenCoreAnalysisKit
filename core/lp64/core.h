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

#ifndef CORE_LP64_CORE_H_
#define CORE_LP64_CORE_H_

#include "api/core.h"
#include <functional>

namespace lp64 {

class Auxv {
public:
    uint64_t type;
    uint64_t value;
};

class File {
public:
    uint64_t begin;
    uint64_t end;
    uint64_t offset;
};

class Debug {
public:
    uint64_t version;
    uint64_t map;
};

class LinkMap {
public:
    uint64_t addr;
    uint64_t name;
    uint64_t ld;
    uint64_t next;
    uint64_t prev;
};

class Dynamic {
public:
    uint64_t type;
    uint64_t value;
};

class Core {
public:
    bool load64(CoreApi* api, std::function<void* (uint64_t, uint64_t)> callback);
    uint64_t loadDebug64(CoreApi* api);
    void loadLinkMap64(CoreApi* api);
    static uint64_t FindDynamic(uint64_t load, uint64_t phdr, uint64_t type);
};

} // namespace lp64

#endif // CORE_LP64_CORE_H_
