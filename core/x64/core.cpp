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

#include "x64/core.h"
#include <linux/elf.h>

namespace x64 {

bool Core::load() {
    auto callback = [](uint64_t type, uint64_t pos) -> void * {
        return nullptr;
    };
    return load64(this, callback);
}

void Core::unload() {
}

uint64_t Core::dlsym(LinkMap* handle, const char* symbol) {
    LoadBlock* block = handle->block();
    uint64_t value = -1;
    if (block) {
        if (!block->isMmapBlock()) {
            value = DynamicSymbol(handle, symbol);
        } else {
            value = dlsym64(block->name().c_str(), symbol);
        }
    }
    if (value != -1) return handle->begin() + value;
    return 0x0;
}

Core::~Core() {
}

} // namespace x64
