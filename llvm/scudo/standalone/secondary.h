/*
 * Copyright (C) 2026-present, Guanyou.Chen. All rights reserved.
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

#ifndef SCUDO_SECONDARY_H_
#define SCUDO_SECONDARY_H_

#include "api/memory_ref.h"

struct scudo_LargeBlock_Header_OffsetTable {
    uint32_t __Prev__;
    uint32_t __Next__;
    uint32_t __CommitBase__;
    uint32_t __CommitSize__;
    uint32_t __MapBase__;
    uint32_t __MapCapacity__;
};

struct scudo_LargeBlock_Header_SizeTable {
    uint32_t THIS;
};

extern struct scudo_LargeBlock_Header_OffsetTable __scudo_LargeBlock_Header_offset__;
extern struct scudo_LargeBlock_Header_SizeTable __scudo_LargeBlock_Header_size__;

namespace scudo {
namespace LargeBlock {

class Header : public api::MemoryRef {
public:
    Header(uint64_t v) : api::MemoryRef(v) {}
    Header(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Header(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Header(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t Prev() { return VALUEOF(scudo_LargeBlock_Header, __Prev__); }
    inline uint64_t Next() { return VALUEOF(scudo_LargeBlock_Header, __Next__); }
    inline uint64_t CommitBase() { return VALUEOF(scudo_LargeBlock_Header, __CommitBase__); }
    inline uint64_t CommitSize() { return VALUEOF(scudo_LargeBlock_Header, __CommitSize__); }
    inline uint64_t MapBase() { return VALUEOF(scudo_LargeBlock_Header, __MapBase__); }
    inline uint64_t MapCapacity() { return VALUEOF(scudo_LargeBlock_Header, __MapCapacity__); }
};

} // namespace LargeBlock
} // namespace scudo

#endif  // SCUDO_SECONDARY_H_
