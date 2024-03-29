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

#ifndef CORE_ARM_CORE_H_
#define CORE_ARM_CORE_H_

#include "api/core.h"
#include "lp32/core.h"
#include "common/elf.h"

namespace arm {

class Core : public CoreApi, lp32::Core {
public:
    Core(std::unique_ptr<MemoryMap>& map)
        : CoreApi(map) {}
    ~Core();
private:
    bool load();
    void unload();
    const char* getMachineName() { return "arm"; }
    int getMachine() { return EM_ARM; }
    int getPointSize() { return 32; }
    uint64_t getVabitsMask() { return 0xFFFFFFFFULL; }
    void loadLinkMap() { loadLinkMap32(this); }
    void sysroot(uint64_t begin, const char* file) { dlopen32(this, begin, file); }
    uint64_t dlsym(LinkMap* handle, const char* symbol);
};

} // namespace arm

#endif // CORE_ARM_CORE_H_
