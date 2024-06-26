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

#ifndef CORE_COMMON_FILE_H_
#define CORE_COMMON_FILE_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <iostream>

class File {
public:
    inline static std::string FIRST_STAGE_RAMDIS = "/first_stage_ramdisk";

    inline uint64_t begin() { return mBegin; }
    inline uint64_t end() { return mEnd; }
    inline uint64_t offset() { return mOffset; }
    inline std::string& name() { return mName; }
    inline bool contains(uint64_t vaddr) { return (vaddr >= mBegin && vaddr < mEnd); }

    File(uint64_t b, uint64_t e, uint64_t off, const char* name)
            : mBegin(b), mEnd(e), mOffset(off) {
        mName = name;
        std::size_t index = mName.find(FIRST_STAGE_RAMDIS);
        if (index != std::string::npos)
            mName = mName.substr(FIRST_STAGE_RAMDIS.length());
    }
    ~File() {}
private:
    //  file member
    uint64_t mBegin;
    uint64_t mEnd;
    uint64_t mOffset;
    std::string mName;
};

#endif  // CORE_COMMON_FILE_H_
