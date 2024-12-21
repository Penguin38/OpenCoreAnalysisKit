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
#include "common/note_block.h"
#include "base/utils.h"

void NoteBlock::addAuxvItem(uint64_t addr, uint64_t type, uint64_t value) {
    std::unique_ptr<Auxv> auxv = std::make_unique<Auxv>(type, value);
    auxv->bind(this, addr);
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
        api->Bind(this);
        mThread.push_back(std::move(api));
    }
}

bool NoteBlock::newOverlay() {
    if (!mOverlay) {
        std::unique_ptr<MemoryMap> map(MemoryMap::MmapMem(begin(), realSize()));
        if (map) {
            mOverlay = std::move(map);
            LOGI("New note overlay [%" PRIx64 ", %" PRIx64 ")\n", offset(), offset() + realSize());
        }
    }
    return mOverlay != 0x0;
}

void NoteBlock::setOverlay(uint64_t addr, void *buf, uint64_t size) {
    if (newOverlay())
        memcpy(reinterpret_cast<uint64_t *>(mOverlay->data() + (addr - oraddr())), buf, size);
}

void NoteBlock::removeOverlay() {
    if (mOverlay) {
        LOGI("Remove note overlay [%" PRIx64 ", %" PRIx64 ")\n", offset(), offset() + realSize());
        mOverlay.reset();
    }
}

NoteBlock::~NoteBlock() {
    mThread.clear();
    mAuxv.clear();
    mFile.clear();
}
