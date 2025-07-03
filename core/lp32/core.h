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
#include "api/elf.h"
#include "base/memory_map.h"
#include <functional>

namespace lp32 {

class Auxv {
public:
    Auxv() : type(0), value(0) {}
    void init(uint32_t t, uint32_t v) {
        type = t;
        value = v;
    }
    uint32_t type;
    uint32_t value;
};

class File {
public:
    File() : begin(0), end(0), offset(0) {}
    uint32_t begin;
    uint32_t end;
    uint32_t offset;
};

class Debug {
public:
    Debug() : version(0), map(0) {}
    uint32_t version;
    uint32_t map;
};

class LinkMap {
public:
    LinkMap() : addr(0), name(0),
                ld(0), next(0), prev(0) {}
    uint32_t addr;
    uint32_t name;
    uint32_t ld;
    uint32_t next;
    uint32_t prev;
};

class Dynamic {
public:
    Dynamic() : type(0), value(0) {}
    uint32_t type;
    uint32_t value;
};

class Core : public api::Elf {
public:
    struct OptionArgs {
        uint64_t type;
        uint64_t pos;
        void *context;
    };
    bool load32(CoreApi* api, std::function<void* (OptionArgs& args)> callback);
    void loadLinkMap32(CoreApi* api);
    bool exec32(CoreApi* api, uint32_t phdr, const char* file);
    bool dlopen32(CoreApi* api, ::LinkMap* handle, const char* file, const char* subfile);
    static void readsym32(::LinkMap* handle);
private:
    bool loader_dlopen32(CoreApi* api, MemoryMap* map, ::LinkMap* handle, uint32_t addr, const char* file);
};

} // namespace lp32

#endif // CORE_LP32_CORE_H_
