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

#ifndef CORE_COMMON_XZ_LZMA_H_
#define CORE_COMMON_XZ_LZMA_H_

#include "common/xz/codec.h"

namespace xz {

class LZMA : public Codec {
public:
    static constexpr uint8_t kMagic[] = { 0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00 };
    LZMA(uint8_t *data, uint64_t size) : Codec(data, size) {}
    ~LZMA() {}

    uint64_t TotalSize();
    MemoryMap* Decode2Map();
};

} // xz

#endif // CORE_COMMON_XZ_LZMA_H_
