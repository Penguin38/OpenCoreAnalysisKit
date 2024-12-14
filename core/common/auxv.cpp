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
#include "common/auxv.h"
#include "common/note_block.h"

void Auxv::bind(NoteBlock* block, uint64_t addr) {
    mBlock = block;
    mAuxv = addr;
}

void Auxv::setValue(uint64_t value) {
    if (mBlock)
        mBlock->setOverlay(mAuxv + CoreApi::GetPointSize(), (void *)&value, CoreApi::GetPointSize());
}
