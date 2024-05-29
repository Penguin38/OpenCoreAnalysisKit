/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "api/core.h"
#include "command/fake/core/lp64/restore.h"
#include <stdio.h>
#include <linux/elf.h>

namespace lp64 {

int Restore::execute(const char* output) {
    if (!CoreApi::IsReady())
        return -1;

    FILE *fp = fopen(output, "wb");
    if (!fp) {
        LOGE("ERROR: Can't open \"%s\".\n", output);
        return -1;
    }

    Elf64_Ehdr *ehdr = reinterpret_cast<Elf64_Ehdr *>(CoreApi::GetBegin());
    Elf64_Phdr *phdr = reinterpret_cast<Elf64_Phdr *>(CoreApi::GetBegin() + ehdr->e_phoff);

    // Write ELF Header
    fwrite(ehdr, sizeof(Elf64_Ehdr), 1, fp);

    // Write Program Headers
    for (int num = 0; num < ehdr->e_phnum; ++num) {
        if (phdr[num].p_type == PT_LOAD) {
            LoadBlock* block = CoreApi::FindLoadBlock(phdr[num].p_vaddr, false);
            if (block) {

            } else {

            }
        } else if (phdr[num].p_type == PT_NOTE) {

        }
    }

    // Write Segment

    fclose(fp);
    LOGI("FakeCore: saved [%s]\n", output);
    return 0;
}

} // namespace lp64
