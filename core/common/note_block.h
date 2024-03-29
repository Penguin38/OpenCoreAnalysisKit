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

#ifndef CORE_COMMON_NOTE_BLOCK_H_
#define CORE_COMMON_NOTE_BLOCK_H_

#include "common/block.h"
#include "common/auxv.h"
#include "common/file.h"
#include "api/thread.h"
#include <memory>
#include <vector>

class NoteBlock : public Block {
public:
    NoteBlock(uint32_t f, uint64_t off, uint64_t va, uint64_t pa,
            uint64_t filesz, uint64_t memsz, uint64_t align)
            : Block(f, off, va, pa, filesz, memsz, align) {}

    void addAuxvItem(uint64_t type, uint64_t value);
    void addFileItem(uint64_t begin, uint64_t end, uint64_t offset, uint64_t pos);
    void addThreadItem(void *thread);
    std::vector<std::unique_ptr<Auxv>>& getAuxv() { return mAuxv; }
    std::vector<std::unique_ptr<File>>& getFile() { return mFile; }
    std::vector<std::unique_ptr<ThreadApi>>& getThread() { return mThread; }
    ~NoteBlock();
private:
    std::vector<std::unique_ptr<ThreadApi>> mThread;
    std::vector<std::unique_ptr<Auxv>> mAuxv;
    std::vector<std::unique_ptr<File>> mFile;
};

#endif  // CORE_COMMON_NOTE_BLOCK_H_
