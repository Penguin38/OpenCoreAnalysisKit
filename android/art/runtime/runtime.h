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

#ifndef ANDROID_ART_RUNTIME_RUNTIME_H_
#define ANDROID_ART_RUNTIME_RUNTIME_H_

#include "logger/log.h"
#include "api/memory_ref.h"
#include "runtime/gc/heap.h"
#include "runtime/thread_list.h"
#include "runtime/class_linker.h"
#include "runtime/art_method.h"
#include "runtime/jni/java_vm_ext.h"
#include "runtime/base/callee_save_type.h"

struct Runtime_OffsetTable {
    uint32_t callee_save_methods_;
    uint32_t resolution_method_;
    uint32_t imt_conflict_method_;
    uint32_t imt_unimplemented_method_;
    uint32_t heap_;
    uint32_t thread_list_;
    uint32_t class_linker_;
    uint32_t java_vm_;
};

struct Runtime_SizeTable {
    uint32_t THIS;
};

extern struct Runtime_OffsetTable __Runtime_offset__;
extern struct Runtime_SizeTable __Runtime_size__;

namespace art {

class Runtime : public api::MemoryRef {
public:
    Runtime() : api::MemoryRef() {}
    Runtime(uint64_t v) : api::MemoryRef(v) {}
    Runtime(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    Runtime(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    Runtime(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    Runtime(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}
    template<typename U> Runtime(U *v) : api::MemoryRef(v) {}
    template<typename U> Runtime(U *v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(Runtime& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Runtime& ref) { return Ptr() != ref.Ptr(); }

    static void Init29();
    static void Init();
    static void Init31();
    static void Init33();
    static void Init34();
    inline uint64_t callee_save_methods() { return Ptr() + OFFSET(Runtime, callee_save_methods_); }
    inline uint64_t resolution_method() { return VALUEOF(Runtime, resolution_method_); }
    inline uint64_t imt_conflict_method() { return VALUEOF(Runtime, imt_conflict_method_); }
    inline uint64_t imt_unimplemented_method() { return VALUEOF(Runtime, imt_unimplemented_method_); }
    inline uint64_t heap() { return VALUEOF(Runtime, heap_); }
    inline uint64_t thread_list() { return VALUEOF(Runtime, thread_list_); }
    inline uint64_t class_linker() { return VALUEOF(Runtime, class_linker_); }
    inline uint64_t java_vm() { return VALUEOF(Runtime, java_vm_); }

    static Runtime& Current();
    gc::Heap& GetHeap();
    ThreadList& GetThreadList();
    ClassLinker& GetClassLinker();
    JavaVMExt& GetJavaVM();
    ArtMethod& GetResolutionMethod();
    ArtMethod& GetImtConflictMethod();
    ArtMethod& GetImtUnimplementedMethod();
    ArtMethod& GetCalleeSaveMethod(CalleeSaveType type);
    ArtMethod& GetCalleeSaveMethodUnchecked(CalleeSaveType type);

    void CleanCache() {
        if (heap_cache.Ptr()) heap_cache.CleanCache();
        if (thread_list_cache.Ptr()) thread_list_cache.CleanCache();
        if (class_linker_cache.Ptr()) class_linker_cache.CleanCache();
    }
private:
    static Runtime AnalysisInstance();
    // quick memoryref cache
    ArtMethod callee_save_methods_cache[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    ArtMethod resolution_method_cache = 0x0;
    ArtMethod imt_conflict_method_cache = 0x0;
    ArtMethod imt_unimplemented_method_cache = 0x0;
    gc::Heap heap_cache = 0x0;
    ThreadList thread_list_cache = 0x0;
    ClassLinker class_linker_cache = 0x0;
    JavaVMExt java_vm_cache = 0x0;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_RUNTIME_H_
