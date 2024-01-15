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

#include "note_block.h"

void NoteBlock::addAuxvItem(uint64_t type, uint64_t value) {
    std::unique_ptr<Auxv> auxv = std::make_unique<Auxv>(type, value);
    mAuxv.push_back(std::move(auxv));
}

void NoteBlock::addFileItem(uint64_t begin, uint64_t end, uint64_t offset, uint64_t pos) {
    const char* name = reinterpret_cast<const char*>(pos);
    std::unique_ptr<File> file = std::make_unique<File>(begin, end, offset << 12, name);
    mFile.push_back(std::move(file));
}

void NoteBlock::addThreadItem(void *thread) {
    if (thread) {
        std::unique_ptr<ThreadApi> api(reinterpret_cast<ThreadApi *>(thread));
        mThread.push_back(std::move(api));
    }
}

NoteBlock::~NoteBlock() {
    mThread.clear();
    mAuxv.clear();
    mFile.clear();
}
