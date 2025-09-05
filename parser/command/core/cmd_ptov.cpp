/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#include "logger/log.h"
#include "base/utils.h"
#include "common/exception.h"
#include "command/core/cmd_ptov.h"
#include "command/core/cmd_vtor.h"
#include "api/core.h"

int PtovCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !(argc > 1))
        return 0;

    options.paddr = Utils::atol(argv[1]);
    PhysicalToRealVma(options);
    return 0;
}

int PtovCommand::PhysicalToRealVma(PtovCommand::Options& options) {
    uint64_t paddr = options.paddr;
    auto callback = [paddr](LoadBlock *block) -> bool {
        if (!block->isValidBlock())
            return false;

        if (paddr >= block->offset() && paddr < block->offset() + block->realSize()) {
            VtorCommand::Options vtor_opt = {
                .vaddr = block->vaddr() + (paddr - block->offset()),
            };
            VtorCommand::VirtualToRealVma(vtor_opt);
            return true;
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, false, false);
    return 0;
}

void PtovCommand::usage() {
    LOGI("Usage: ptov <CORE_OFFSET>\n");
}
