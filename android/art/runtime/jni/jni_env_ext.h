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

#ifndef ANDROID_ART_RUNTIME_JNI_JNI_ENV_EXT_H_
#define ANDROID_ART_RUNTIME_JNI_JNI_ENV_EXT_H_

#include "api/memory_ref.h"
#include "runtime/mirror/object.h"
#include "runtime/jni/local_reference_table.h"
#include "runtime/indirect_reference_table.h"

struct JNIEnvExt_OffsetTable {
    uint32_t functions;
    uint32_t self_;
    uint32_t vm_;
    uint32_t locals_;
};

extern struct JNIEnvExt_OffsetTable __JNIEnvExt_offset__;

namespace art {

class JNIEnvExt : public api::MemoryRef {
public:
    JNIEnvExt(uint64_t v) : api::MemoryRef(v) {}
    JNIEnvExt(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    JNIEnvExt(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    JNIEnvExt(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static void Init23();
    static void Init35();
    inline uint64_t functions() { return VALUEOF(JNIEnvExt, functions); }
    inline uint64_t self() { return VALUEOF(JNIEnvExt, self_); }
    inline uint64_t vm() { return VALUEOF(JNIEnvExt, vm_); }
    inline uint64_t locals() { return Ptr() + OFFSET(JNIEnvExt, locals_); }
    inline uint64_t locals_v34() { return locals(); }

    mirror::Object Decode(uint64_t uref);
    mirror::Object DecodeLocal(uint64_t uref);
    inline IndirectReferenceTable& GetLocalsTable() { return QUICK_CACHE(locals); }
    inline jni::LocalReferenceTable& GetLocalsTable_v34() { return QUICK_CACHE(locals_v34); }
private:
    DEFINE_QUICK_CACHE(IndirectReferenceTable, locals);
    DEFINE_QUICK_CACHE(jni::LocalReferenceTable, locals_v34);
};

} // namespace art

#endif  // ANDROID_ART_RUNTIME_JNI_JNI_ENV_EXT_H_
