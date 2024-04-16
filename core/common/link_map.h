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

#ifndef CORE_COMMON_LINKMAP_H_
#define CORE_COMMON_LINKMAP_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>
#include <iostream>

class LinkMap {
public:
    inline uint64_t map() { return mMap; }
    inline uint64_t begin() { return mBegin; }
    inline std::string& name() { return mName; }
    inline LoadBlock* block() { return mBlock; }

    LinkMap(uint64_t m, uint64_t b, const char* name, LoadBlock* block)
            : mMap(m), mBegin(b) {
        if (name) mName = name;
        mBlock = block;
    }
    ~LinkMap() {}
private:
    uint64_t mMap;
    //  file member
    uint64_t mBegin;
    std::string mName;
    LoadBlock* mBlock;
};

#endif  // CORE_COMMON_LINKMAP_H_
