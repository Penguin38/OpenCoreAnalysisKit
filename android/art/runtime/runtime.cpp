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

#include "runtime/runtime.h"
#include "runtime/image.h"
#include "runtime/entrypoints/quick/callee_save_frame.h"
#include "android.h"
#include "common/exception.h"
#include <string.h>

struct Runtime_OffsetTable __Runtime_offset__;
struct Runtime_SizeTable __Runtime_size__;

namespace art {

uint32_t Runtime::kNumCalleeMethodsCount = 3;

void Runtime::Init() {
    Android::RegisterSdkListener(Android::M, art::Runtime::Init23);
    Android::RegisterSdkListener(Android::N, art::Runtime::Init24);
    Android::RegisterSdkListener(Android::O, art::Runtime::Init26);
    Android::RegisterSdkListener(Android::P, art::Runtime::Init28);
    Android::RegisterSdkListener(Android::Q, art::Runtime::Init29);
    Android::RegisterSdkListener(Android::R, art::Runtime::Init30);
    Android::RegisterSdkListener(Android::S, art::Runtime::Init31);
    Android::RegisterSdkListener(Android::T, art::Runtime::Init33);
    Android::RegisterSdkListener(Android::U, art::Runtime::Init34);
}

void Runtime::Init23() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 32,
            .imt_conflict_method_ = 40,
            .imt_unimplemented_method_ = 48,
            .heap_ = 320,
            .monitor_pool_ = 368,
            .thread_list_ = 376,
            .class_linker_ = 392,
            .java_vm_ = 432,
            .jit_ = 440,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 32,
            .imt_conflict_method_ = 36,
            .imt_unimplemented_method_ = 40,
            .heap_ = 200,
            .monitor_pool_ = 224,
            .thread_list_ = 228,
            .class_linker_ = 236,
            .java_vm_ = 256,
            .jit_ = 260,
        };
    }

    kNumCalleeMethodsCount = 3;
}

void Runtime::Init24() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 32,
            .imt_conflict_method_ = 40,
            .imt_unimplemented_method_ = 48,
            .heap_ = 320,
            .monitor_pool_ = 376,
            .thread_list_ = 384,
            .class_linker_ = 400,
            .java_vm_ = 440,
            .jit_ = 448,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 32,
            .imt_conflict_method_ = 40,
            .imt_unimplemented_method_ = 48,
            .heap_ = 200,
            .monitor_pool_ = 228,
            .thread_list_ = 232,
            .class_linker_ = 240,
            .java_vm_ = 260,
            .jit_ = 264,
        };
    }

    kNumCalleeMethodsCount = 3;
}

void Runtime::Init26() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 40,
            .imt_conflict_method_ = 48,
            .imt_unimplemented_method_ = 56,
            .heap_ = 384,
            .monitor_pool_ = 440,
            .thread_list_ = 448,
            .class_linker_ = 464,
            .java_vm_ = 504,
            .jit_ = 512,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 40,
            .imt_conflict_method_ = 48,
            .imt_unimplemented_method_ = 56,
            .heap_ = 244,
            .monitor_pool_ = 272,
            .thread_list_ = 276,
            .class_linker_ = 284,
            .java_vm_ = 304,
            .jit_ = 308,
        };
    }

    kNumCalleeMethodsCount = 4;
}

void Runtime::Init28() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 56,
            .imt_conflict_method_ = 64,
            .imt_unimplemented_method_ = 72,
            .heap_ = 448,
            .monitor_pool_ = 504,
            .thread_list_ = 512,
            .class_linker_ = 528,
            .java_vm_ = 576,
            .jit_ = 584,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 56,
            .imt_conflict_method_ = 64,
            .imt_unimplemented_method_ = 72,
            .heap_ = 296,
            .monitor_pool_ = 324,
            .thread_list_ = 328,
            .class_linker_ = 336,
            .java_vm_ = 360,
            .jit_ = 304,
        };
    }

    kNumCalleeMethodsCount = 6;
}

void Runtime::Init29() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 400,
            .monitor_pool_ = 456,
            .thread_list_ = 464,
            .class_linker_ = 480,
            .java_vm_ = 496,
            .jit_ = 504,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 240,
            .monitor_pool_ = 268,
            .thread_list_ = 272,
            .class_linker_ = 280,
            .java_vm_ = 288,
            .jit_ = 292,
        };
    }

    kNumCalleeMethodsCount = 6;
}

void Runtime::Init30() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 392,
            .monitor_pool_ = 448,
            .thread_list_ = 456,
            .class_linker_ = 472,
            .java_vm_ = 496,
            .jit_ = 504,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 236,
            .monitor_pool_ = 264,
            .thread_list_ = 268,
            .class_linker_ = 276,
            .java_vm_ = 288,
            .jit_ = 292,
        };
    }

    kNumCalleeMethodsCount = 6;
}

void Runtime::Init31() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 416,
            .monitor_pool_ = 472,
            .thread_list_ = 480,
            .class_linker_ = 496,
            .java_vm_ = 520,
            .jit_ = 528,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 248,
            .monitor_pool_ = 276,
            .thread_list_ = 280,
            .class_linker_ = 288,
            .java_vm_ = 300,
            .jit_ = 304,
        };
    }

    kNumCalleeMethodsCount = 6;
}

void Runtime::Init33() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 512,
            .monitor_pool_ = 568,
            .thread_list_ = 576,
            .class_linker_ = 592,
            .java_vm_ = 624,
            .jit_ = 632,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 296,
            .monitor_pool_ = 324,
            .thread_list_ = 328,
            .class_linker_ = 336,
            .java_vm_ = 352,
            .jit_ = 356,
        };
    }

    kNumCalleeMethodsCount = 6;
}

void Runtime::Init34() {
    if (CoreApi::Bits() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 72,
            .imt_unimplemented_method_ = 80,
            .heap_ = 512,
            .monitor_pool_ = 576,
            .thread_list_ = 584,
            .class_linker_ = 600,
            .java_vm_ = 632,
            .jit_ = 640,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .resolution_method_ = 64,
            .imt_conflict_method_ = 68,
            .imt_unimplemented_method_ = 72,
            .heap_ = 296,
            .monitor_pool_ = 328,
            .thread_list_ = 332,
            .class_linker_ = 340,
            .java_vm_ = 356,
            .jit_ = 360,
        };
    }

    kNumCalleeMethodsCount = 6;
}

api::MemoryRef Runtime::runtime_instance_ori_cache = 0x0;

api::MemoryRef& Runtime::Origin() {
    return runtime_instance_ori_cache;
}

Runtime& Runtime::Current() {
    Runtime& runtime = Android::GetRuntime();
    if (!Android::IsSdkReady()) {
        LOGE("Please command \"env config --sdk <SDK>!!\"\n");
        if (runtime.Ptr()) {
            runtime.CleanCache();
            runtime = 0x0;
        }
        return runtime;
    }

    if (!runtime.Ptr()) {
        api::MemoryRef value = 0x0;
        try {
            value = Android::DlSym(Android::ART_RUNTIME_INSTANCE);
            runtime_instance_ori_cache = value.valueOf();
            if (!runtime_instance_ori_cache.IsValid())
                throw InvalidAddressException(runtime_instance_ori_cache.Ptr());
            runtime = runtime_instance_ori_cache;

#if defined(__ART_RUNTIME_PARSER__)
            // double check runtime callee_method
            uint64_t callee_methods[6] = {0x0};
            memset(callee_methods, 0x0, sizeof(callee_methods));
            uint32_t sizeof_callee_methods = SizeOfCalleeMethods();
            AnalysisCalleeSaveMethods(callee_methods, sizeof_callee_methods);

            //maybe invalid
            if (!callee_methods[0])
                return runtime;

            if (memcmp(reinterpret_cast<void *>(callee_methods),
                        reinterpret_cast<void *>(runtime.Real()),
                        sizeof_callee_methods)) {
                LOGW("'%s' = 0x%" PRIx64 ", but callee_save_methods_ not match image.\n",
                        Android::ART_RUNTIME_INSTANCE, runtime.Ptr());
                runtime = AnalysisRuntime(callee_methods, sizeof_callee_methods);
            }
#endif
        } catch(InvalidAddressException e) {
            runtime = AnalysisInstance();
        }
    }
    return runtime;
}

void Runtime::AnalysisCalleeSaveMethods(uint64_t *callee_methods, uint32_t sizeof_callee_methods) {
    if (!callee_methods)
        return;

    auto callback = [&](LoadBlock *block) -> bool {
        if (memcmp(reinterpret_cast<void *>(block->begin()),
                   reinterpret_cast<void *>(ImageHeader::kMagic),
                   sizeof(ImageHeader::kMagic)))
            return false;

        ImageHeader header(block->vaddr(), block);
        void *image_callee_methods = reinterpret_cast<void *>(header.image_methods()
                + ImageHeader::ImageMethod::kSaveAllCalleeSavesMethod * sizeof(uint64_t));

        if (!reinterpret_cast<uint64_t *>(image_callee_methods)[0])
            return false;

        memcpy(callee_methods, image_callee_methods, sizeof_callee_methods);
        return true;
    };
    CoreApi::ForeachLoadBlock(callback, true, true);
}

Runtime Runtime::AnalysisRuntime(uint64_t *callee_methods, uint32_t sizeof_callee_methods) {
    Runtime runtime = 0x0;
    uint64_t point_size = CoreApi::GetPointSize();
    auto match = [&](LoadBlock *block) -> bool {
        // must can write vma
        if (!(block->flags() & Block::FLAG_W))
            return false;

        if (!memcmp(reinterpret_cast<void *>(block->begin()),
                   reinterpret_cast<void *>(ImageHeader::kMagic),
                   sizeof(ImageHeader::kMagic)))
            return false;

        uint64_t current = block->begin();
        uint64_t outsize = block->begin() + block->size();
        while (current + sizeof_callee_methods < outsize) {
            if (!memcmp(reinterpret_cast<void *>(callee_methods),
                        reinterpret_cast<void *>(current),
                        sizeof_callee_methods)) {
                runtime = current - block->begin() + block->vaddr();
                runtime.checkCopyBlock(block);
                ArtMethod& resolution_method_ = runtime.GetResolutionMethod();
                if (resolution_method_.Block() &&
                        resolution_method_.Block()->virtualContains(callee_methods[0])) {
                    LOGD(">>> '%s' = 0x%" PRIx64 "\n", Android::ART_RUNTIME_INSTANCE, runtime.Ptr());
                    return true;
                }
            }
            current += point_size;
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(match, true, true);
    return runtime;
}

Runtime Runtime::AnalysisInstance() {
    uint64_t callee_methods[6] = {0x0};
    memset(callee_methods, 0x0, sizeof(callee_methods));
    uint32_t sizeof_callee_methods = SizeOfCalleeMethods();
    AnalysisCalleeSaveMethods(callee_methods, sizeof_callee_methods);

    //maybe invalid
    if (!callee_methods[0])
        return 0x0;

    return AnalysisRuntime(callee_methods, sizeof_callee_methods);
}

gc::Heap& Runtime::GetHeap() {
    if (!heap_cache.Ptr()) {
        heap_cache = heap();
        heap_cache.copyRef(this);
        heap_cache.Prepare(false);
    }
    return heap_cache;
}

MonitorPool& Runtime::GetMonitorPool() {
    if (!monitor_pool_cache.Ptr()) {
        monitor_pool_cache = monitor_pool();
        monitor_pool_cache.copyRef(this);
        monitor_pool_cache.Prepare(false);
    }
    return monitor_pool_cache;
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

jit::Jit& Runtime::GetJit() {
    if (!jit_cache.Ptr()) {
        jit_cache = jit();
        jit_cache.copyRef(this);
        jit_cache.Prepare(false);
    }
    return jit_cache;
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
        memset(callee_save_methods_cache, 0x0, sizeof(callee_save_methods_cache));
        api::MemoryRef ref = callee_save_methods();
        for (uint32_t i = 0; i < kNumCalleeMethodsCount; ++i) {
            callee_save_methods_cache[i] = ref.value64Of(i * sizeof(uint64_t));
            callee_save_methods_cache[i].Prepare(false);
        }
    }
    return callee_save_methods_cache[index];
}

QuickMethodFrameInfo Runtime::GetRuntimeMethodFrameInfo(ArtMethod& method) {
    if (method == GetCalleeSaveMethodUnchecked(CalleeSaveType::kSaveRefsAndArgs)) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
    } else if (method == GetCalleeSaveMethodUnchecked(CalleeSaveType::kSaveAllCalleeSaves)) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveAllCalleeSaves);
    } else if (method == GetCalleeSaveMethodUnchecked(CalleeSaveType::kSaveRefsOnly)) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsOnly);
    } else {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveEverything);
    }
}

} // namespace art
