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

#include "api/core.h"
#include "note_block.h"
#include "common/bit.h"
#include <linux/elf.h>

void NoteBlock::parseNote() {
    switch(CoreApi::GetPointSize()) {
        case 64: parseNote64(); break;
        case 32: parseNote32(); break;
    }
}

void NoteBlock::parseNote64() {
    std::cout << __func__ << std::endl;
    if (isValidBlock()) {
        uint64_t pos = oraddr();
        uint64_t end = oraddr() + realSize();
        while (pos < end) {
            Elf64_Nhdr *nhdr = reinterpret_cast<Elf64_Nhdr *>(pos);
            pos = pos + RoundUp(nhdr->n_descsz, 0x4) + sizeof(Elf64_Nhdr) + sizeof(uint64_t);
        }
    }
}

void NoteBlock::parseNote32() {
    std::cout << __func__ << std::endl;
}
