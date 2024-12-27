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

#ifndef CORE_COMMON_XZ_CODEC_H_
#define CORE_COMMON_XZ_CODEC_H_

#include "base/memory_map.h"
#include <stdint.h>
#include <sys/types.h>
#include <memory>

namespace xz {

class Codec {
public:
    static bool HasLZMASupport();
    inline uint8_t* data() { return mData; }
    inline uint64_t size() { return mSize; }

    Codec(uint8_t *data, uint64_t size)
        : mData(data), mSize(size) {}

    virtual ~Codec() {}
    virtual MemoryMap* Decode2Map() = 0;

    static bool IsLZMA(uint8_t* data);
    static std::unique_ptr<Codec> Create(uint8_t *data, uint64_t size);
private:
    uint8_t *mData;
    uint64_t mSize;
};

} // xz

#endif // CORE_COMMON_XZ_CODEC_H_
