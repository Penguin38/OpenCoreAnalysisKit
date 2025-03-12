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
#include "command/core/cmd_vtor.h"
#include "api/core.h"

int VtorCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !(argc > 1))
        return 0;

    uint64_t vaddr = Utils::atol(argv[1]);
    LoadBlock* block = CoreApi::FindLoadBlock(vaddr, false, false);
    if (!block)
        return 0;

    try {
        LOGI("  * OR: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, CoreApi::GetReal(vaddr, OPT_READ_OR));
        LOGI("  * MMAP: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, CoreApi::GetReal(vaddr, OPT_READ_MMAP));
        LOGI("  * OVERLAY: " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, CoreApi::GetReal(vaddr, OPT_READ_OVERLAY));
    } catch(InvalidAddressException &e) {}

    std::string name;
    if (block->name().length() > 0) {
        name.append(Logger::Green());
        name.append(block->name());
        name.append(Logger::End());
    } else {
        name.append("[]");
    }
    LOGI(ANSI_COLOR_CYAN "[%" PRIx64 ", %" PRIx64 ")" ANSI_COLOR_RESET "  %s  %010" PRIx64 "  %010" PRIx64 "  %s %s\n",
            block->vaddr(), block->vaddr() + block->memsz(), block->convertFlags().c_str(),
            block->realSize(), block->memsz(), name.c_str(), block->convertValids().c_str());
    return 0;
}

void VtorCommand::usage() {
    LOGI("Usage: vtor <VADDR>\n");
    ENTER();
    LOGI("core-parser> vtor 0x14000000\n");
    LOGI("  * OR: 0x73072868d000\n");
    LOGI("  * MMAP: 0x0\n");
    LOGI("  * OVERLAY: 0x0\n");
    LOGI("[14000000, 38000000)  rw-  0024000000  0024000000  [anon:dalvik-main space] [*]\n");
}
