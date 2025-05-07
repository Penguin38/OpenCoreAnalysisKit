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

#include "x86/core.h"
#include "x86/thread_info.h"
#include "common/prstatus.h"
#include "common/bit.h"
#include <string.h>
#include <linux/elf.h>

namespace x86 {

bool Core::load() {
    pointer_mask = GENMASK_UL(bits() - 1, 0);
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

} // namespace x86
