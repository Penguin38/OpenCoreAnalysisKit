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

#ifndef LLVM_CXX_MUTEX_H_
#define LLVM_CXX_MUTEX_H_

#include "api/memory_ref.h"

struct cxx_mutex_OffsetTable {
    uint32_t __m_;
};

struct cxx_mutex_SizeTable {
    uint32_t THIS;
};

extern struct cxx_mutex_OffsetTable __cxx_mutex_offset__;
extern struct cxx_mutex_SizeTable __cxx_mutex_size__;

namespace cxx {

class mutex : public api::MemoryRef {
public:
    mutex(uint64_t v) : api::MemoryRef(v) {}
    mutex(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    mutex(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    mutex(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(mutex& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(mutex& ref) { return Ptr() != ref.Ptr(); }

    static void Init();
    inline uint64_t __m_() { return Ptr() + OFFSET(cxx_mutex, __m_); }
};

} // namespace cxx

#endif  // LLVM_CXX_MUTEX_H_
