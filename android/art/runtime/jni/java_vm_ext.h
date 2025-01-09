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

#ifndef ANDROID_ART_RUNTIME_JNI_JAVA_VM_EXT_H_
#define ANDROID_ART_RUNTIME_JNI_JAVA_VM_EXT_H_

#include "api/memory_ref.h"
#include "runtime/indirect_reference_table.h"

struct JavaVMExt_OffsetTable {
    uint32_t globals_;
    uint32_t weak_globals_;
};

struct JavaVMExt_SizeTable {
    uint32_t THIS;
};

extern struct JavaVMExt_OffsetTable __JavaVMExt_offset__;
extern struct JavaVMExt_SizeTable __JavaVMExt_size__;

namespace art {

class JavaVMExt : public api::MemoryRef {
public:
    JavaVMExt(uint64_t v) : api::MemoryRef(v) {}
    JavaVMExt(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    JavaVMExt(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    JavaVMExt(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static void Init23();
    static void Init24();
    static void Init26();
    static void Init29();
    static void Init31();
    static void Init33();
    static void Init34();
    inline uint64_t globals() { return Ptr() + OFFSET(JavaVMExt, globals_); }
    inline uint64_t weak_globals() { return Ptr() + OFFSET(JavaVMExt, weak_globals_); }

    IndirectReferenceTable& GetGlobalsTable();
    IndirectReferenceTable& GetWeakGlobalsTable();

    mirror::Object Decode(uint64_t uref);
    mirror::Object DecodeGlobal(uint64_t uref);
    mirror::Object DecodeWeakGlobal(uint64_t uref);
private:
    // quick memoryref cache
    IndirectReferenceTable globals_cache = 0x0;
    IndirectReferenceTable weak_globals_cache = 0x0;
};

} //namespace art

#endif  // ANDROID_ART_RUNTIME_JNI_JAVA_VM_EXT_H_
