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

#include "logger/log.h"
#include "command/llvm/cmd_scudo.h"
#include "scudo/standalone/chunk.h"
#include "scudo/standalone/secondary.h"
#include "api/core.h"
#include "api/memory_ref.h"
#include "base/utils.h"

int ScudoCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !(argc > 1))
        return 0;

    uint64_t address = Utils::atol(argv[1]) & CoreApi::GetVabitsMask();
    api::MemoryRef ref = address - 0x10;
    scudo::Chunk::UnpackedHeader* chunk_header = reinterpret_cast<scudo::Chunk::UnpackedHeader*>(ref.Real());
    LOGI("scudo::Chunk::UnpackedHeader (" ANSI_COLOR_LIGHTMAGENTA "0x%lx" ANSI_COLOR_RESET ")\n", ref.Ptr());
    LOGI("  * ClassId: " ANSI_COLOR_LIGHTMAGENTA "0x%0x" ANSI_COLOR_RESET "\n", chunk_header->ClassId);
    LOGI("  * State: " ANSI_COLOR_LIGHTMAGENTA "0x%x" ANSI_COLOR_RESET "\n", chunk_header->State);
    LOGI("  * OriginOrWasZeroed: " ANSI_COLOR_LIGHTMAGENTA "0x%x" ANSI_COLOR_RESET "\n", chunk_header->OriginOrWasZeroed);
    LOGI("  * SizeOrUnusedBytes: " ANSI_COLOR_LIGHTMAGENTA "0x%x" ANSI_COLOR_RESET "\n", chunk_header->SizeOrUnusedBytes);
    LOGI("  * Offset: " ANSI_COLOR_LIGHTMAGENTA "0x%x" ANSI_COLOR_RESET "\n", chunk_header->Offset);
    LOGI("  * Checksum: " ANSI_COLOR_LIGHTMAGENTA "0x%x" ANSI_COLOR_RESET "\n", chunk_header->Checksum);

    if (chunk_header->ClassId == 0x0) {
        scudo::LargeBlock::Header block = ref.Ptr() - SIZEOF(scudo_LargeBlock_Header);
        LOGI("scudo::LargeBlock::Header (" ANSI_COLOR_LIGHTMAGENTA "0x%lx" ANSI_COLOR_RESET ")\n", block.Ptr());
        LOGI("  * Prev: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n", block.Prev());
        if (block.Prev() != 0x0)
            LOGI("  *   PrevPtr: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n",
                    (block.Prev() + 0x10 + SIZEOF(scudo_LargeBlock_Header)) & CoreApi::GetVabitsMask());
        LOGI("  * Next: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n", block.Next());
        if (block.Next() != 0x0)
            LOGI("  *   NextPtr: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n",
                    (block.Next() + 0x10 + SIZEOF(scudo_LargeBlock_Header)) & CoreApi::GetVabitsMask());
        LOGI("  * CommitBase: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n", block.CommitBase());
        LOGI("  * CommitSize: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n", block.CommitSize());
        LOGI("  * MapBase: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n", block.MapBase());
        LOGI("  * MapCapacity: " ANSI_COLOR_LIGHTMAGENTA "0x%0lx" ANSI_COLOR_RESET "\n", block.MapCapacity());
    }
    return 0;
}

void ScudoCommand::usage() {
    LOGI("Usage: scudo <ADDRESS> [options..]\n");
}
