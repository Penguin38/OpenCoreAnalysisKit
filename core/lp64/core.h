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
#include "api/elf.h"
#include "base/memory_map.h"
#include <functional>

namespace lp64 {

class Auxv {
public:
    Auxv() : type(0), value(0) {}
    void init(uint64_t t, uint64_t v) {
        type = t;
        value = v;
    }
    uint64_t type;
    uint64_t value;
};

class File {
public:
    File() : begin(0), end(0), offset(0) {}
    uint64_t begin;
    uint64_t end;
    uint64_t offset;
};

class Debug {
public:
    Debug() : version(0), map(0) {}
    uint64_t version;
    uint64_t map;
};

class LinkMap {
public:
    LinkMap() : addr(0), name(0),
                ld(0), next(0), prev(0) {}
    uint64_t addr;
    uint64_t name;
    uint64_t ld;
    uint64_t next;
    uint64_t prev;
};

class Dynamic {
public:
    Dynamic() : type(0), value(0) {}
    uint64_t type;
    uint64_t value;
};

class Core : public api::Elf {
public:
    struct OptionArgs {
        uint64_t type;
        uint64_t pos;
        void *context;
    };
    bool load64(CoreApi* api, std::function<void* (OptionArgs& args)> callback);
    void loadLinkMap64(CoreApi* api);
    bool exec64(CoreApi* api, uint64_t phdr, const char* file);
    bool dlopen64(CoreApi* api, ::LinkMap* handle, const char* file, const char* subfile);
    static void readsym64(::LinkMap* handle);
private:
    bool loader_dlopen64(CoreApi* api, MemoryMap* map, ::LinkMap* handle, uint64_t addr, const char* file);
};

} // namespace lp64

#endif // CORE_LP64_CORE_H_
