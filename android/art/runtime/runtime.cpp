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

#include "runtime/runtime.h"
#include "runtime/image.h"
#include "android.h"
#include "common/exception.h"
#include <string.h>

struct Runtime_OffsetTable __Runtime_offset__;
struct Runtime_SizeTable __Runtime_size__;

namespace art {

void Runtime::Init29() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 400,
            .thread_list_ = 464,
            .class_linker_ = 480,
            .java_vm_ = 496,
        };
    } else {
        //TODO
    }
}

void Runtime::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 392,
            .thread_list_ = 456,
            .class_linker_ = 472,
            .java_vm_ = 496,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 236,
            .thread_list_ = 268,
            .class_linker_ = 276,
            .java_vm_ = 288,
        };
    }
}

void Runtime::Init31() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 416,
            .thread_list_ = 480,
            .class_linker_ = 496,
            .java_vm_ = 520,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 248,
            .thread_list_ = 280,
            .class_linker_ = 288,
            .java_vm_ = 300,
        };
    }
}

void Runtime::Init33() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 512,
            .thread_list_ = 576,
            .class_linker_ = 592,
            .java_vm_ = 624,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 296,
            .thread_list_ = 328,
            .class_linker_ = 336,
            .java_vm_ = 352,
        };
    }
}

void Runtime::Init34() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 512,
            .thread_list_ = 584,
            .class_linker_ = 600,
            .java_vm_ = 632,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 296,
            .thread_list_ = 332,
            .class_linker_ = 340,
            .java_vm_ = 356,
        };
    }
}

Runtime& Runtime::Current() {
    Runtime& runtime = Android::GetRuntime();
    if (!runtime.Ptr()) {
        api::MemoryRef value = 0x0;
        try {
            value = Android::SearchSymbol(Android::ART_RUNTIME_INSTANCE);
            runtime = value.valueOf();
        } catch(InvalidAddressException e) {
            runtime = AnalysisInstance();
        }
    }
    return runtime;
}

Runtime Runtime::AnalysisInstance() {
    Runtime runtime = 0x0;
    uint64_t callee_methods[6] = {0x0};
    uint8_t art_magic[4] = {0x61, 0x72, 0x74, 0x0A}; // art\n
    auto callback = [&](LoadBlock *block) -> bool {
        if (memcmp(reinterpret_cast<void *>(block->begin()),
                   reinterpret_cast<void *>(art_magic),
                   sizeof(art_magic)))
            return false;

        ImageHeader header(block->vaddr(), block);
        memcpy(reinterpret_cast<void *>(callee_methods),
               reinterpret_cast<void *>(header.image_methods()
                   + ImageHeader::ImageMethod::kSaveAllCalleeSavesMethod * sizeof(uint64_t)),
               sizeof(callee_methods));

        return true;
    };
    CoreApi::ForeachLoadBlock(callback);

    //maybe invalid
    if (!callee_methods[0])
        return runtime;

    uint64_t point_size = CoreApi::GetPointSize() / 8;
    auto match = [&](LoadBlock *block) -> bool {
        // must can write vma
        if (!(block->flags() & Block::FLAG_W))
            return false;

        if (!memcmp(reinterpret_cast<void *>(block->begin()),
                   reinterpret_cast<void *>(art_magic),
                   sizeof(art_magic)))
            return false;

        uint64_t current = block->begin();
        uint64_t outsize = block->begin() + block->size();
        while (current + sizeof(callee_methods) < outsize) {
            if (!memcmp(reinterpret_cast<void *>(callee_methods),
                        reinterpret_cast<void *>(current),
                        sizeof(callee_methods))) {
                runtime = current - block->begin() + block->vaddr();
                runtime.checkCopyBlock(block);
                LOGI(">>> '%s' = 0x%lx\n", Android::ART_RUNTIME_INSTANCE, runtime.Ptr());
                return true;
            }
            current += point_size;
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(match);
    return runtime;
}

gc::Heap& Runtime::GetHeap() {
    if (!heap_cache.Ptr()) {
        heap_cache = heap();
        heap_cache.copyRef(this);
        heap_cache.Prepare(false);
    }
    return heap_cache;
}

ThreadList& Runtime::GetThreadList() {
    if (!thread_list_cache.Ptr()) {
        thread_list_cache = thread_list();
        thread_list_cache.copyRef(this);
        thread_list_cache.Prepare(false);
    }
    return thread_list_cache;
}

ClassLinker& Runtime::GetClassLinker() {
    if (!class_linker_cache.Ptr()) {
        class_linker_cache = class_linker();
        class_linker_cache.copyRef(this);
        class_linker_cache.Prepare(false);
    }
    return class_linker_cache;
}

JavaVMExt& Runtime::GetJavaVM() {
    if (!java_vm_cache.Ptr()) {
        java_vm_cache = java_vm();
        java_vm_cache.copyRef(this);
        java_vm_cache.Prepare(false);
    }
    return java_vm_cache;
}

ArtMethod& Runtime::GetResolutionMethod() {
    if (!resolution_method_cache.Ptr()) {
        resolution_method_cache = resolution_method();
        resolution_method_cache.Prepare(false);
    }
    return resolution_method_cache;
}

ArtMethod& Runtime::GetImtConflictMethod() {
    if (!imt_conflict_method_cache.Ptr()) {
        imt_conflict_method_cache = imt_conflict_method();
        imt_conflict_method_cache.Prepare(false);
    }
    return imt_conflict_method_cache;
}

ArtMethod& Runtime::GetImtUnimplementedMethod() {
    if (!imt_unimplemented_method_cache.Ptr()) {
        imt_unimplemented_method_cache = imt_unimplemented_method();
        imt_unimplemented_method_cache.Prepare(false);
    }
    return imt_unimplemented_method_cache;
}

ArtMethod& Runtime::GetCalleeSaveMethod(CalleeSaveType type) {
    return GetCalleeSaveMethodUnchecked(type);
}

ArtMethod& Runtime::GetCalleeSaveMethodUnchecked(CalleeSaveType type) {
    uint32_t index = static_cast<uint32_t>(type);
    if (!callee_save_methods_cache[index].Ptr()) {
        api::MemoryRef ref = callee_save_methods();
        for (uint32_t i = 0; i < static_cast<uint32_t>(CalleeSaveType::kLastCalleeSaveType); ++i) {
            callee_save_methods_cache[i] = ref.value64Of(i * sizeof(uint64_t));
            callee_save_methods_cache[i].Prepare(false);
        }
    }
    return callee_save_methods_cache[index];
}

} // namespace art
