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

#ifndef ANDROID_ART_RUNTIME_OAT_OAT_FILE_H_
#define ANDROID_ART_RUNTIME_OAT_OAT_FILE_H_

#include "api/memory_ref.h"
#include "runtime/vdex_file.h"

struct OatFile_OffsetTable {
    uint32_t vdex_;
    uint32_t vdex_begin_;
};
extern struct OatFile_OffsetTable __OatFile_offset__;

struct OatDexFile_OffsetTable {
    uint32_t oat_file_;
};
extern struct OatDexFile_OffsetTable __OatDexFile_offset__;

namespace art {

class OatFile : public api::MemoryRef {
public:
    OatFile(uint64_t v) : api::MemoryRef(v) {}
    OatFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    OatFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> OatFile(U *v) : api::MemoryRef(v) {}
    template<typename U> OatFile(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t vdex() { return VALUEOF(OatFile, vdex_); }
    inline uint64_t vdex_begin() { return VALUEOF(OatFile, vdex_begin_); }

    VdexFile& GetVdexFile();
private:
    // quick memoryref cache
    VdexFile vdex_cache = 0x0;
};

class OatDexFile : public api::MemoryRef {
public:
    OatDexFile(uint64_t v) : api::MemoryRef(v) {}
    OatDexFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    OatDexFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> OatDexFile(U *v) : api::MemoryRef(v) {}
    template<typename U> OatDexFile(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t oat_file() { return VALUEOF(OatDexFile, oat_file_); }

    OatFile& GetOatFile();
private:
    // quick memoryref cache
    OatFile oat_file_cache = 0x0;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_OAT_OAT_FILE_H_
