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

#include "arm/core.h"
#include "arm/thread_info.h"
#include "common/prstatus.h"
#include <string.h>
#include <linux/elf.h>

namespace arm {

bool Core::load() {
    pointer_mask = ((1ULL << (bits() - 1)) - 1) | (1ULL << (bits() - 1));
    vabits_mask = pointer_mask;

    auto callback = [](uint64_t type, uint64_t pos) -> void * {
        switch(type) {
            case NT_PRSTATUS:
                Elf32_prstatus* prs = reinterpret_cast<Elf32_prstatus *>(pos);
                ThreadInfo* thread = new ThreadInfo(prs->pr_pid, pos);
                memcpy(&thread->reg, &prs->pr_reg, sizeof(Register));
                return thread;
        }
        return nullptr;
    };
    return load32(this, callback);
}

void Core::unload() {
}

Core::~Core() {
    unload();
}

} // namespace arm
