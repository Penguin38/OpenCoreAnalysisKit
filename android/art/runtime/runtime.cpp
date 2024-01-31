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

void Runtime::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .heap_ = 392,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .heap_ = 236,
        };
    }
}

void Runtime::Init31() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .heap_ = 416,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .heap_ = 248,
        };
    }
}

void Runtime::Init33() {
    if (CoreApi::GetPointSize() == 64) {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .heap_ = 512,
        };
    } else {
        __Runtime_offset__ = {
            .callee_save_methods_ = 0,
            .heap_ = 296,
        };
    }
}

Runtime& Runtime::Current() {
    Runtime& runtime = Android::GetRuntime();
    if (!runtime.Ptr()) {
        api::MemoryRef value = 0x0;
        try {
            value = Android::SearchSymbol(Android::ART_RUNTIME_INSTANCE);
            if (CoreApi::GetPointSize() == 64) {
                runtime = *reinterpret_cast<uint64_t *>(value.Real());
            } else {
                runtime = *reinterpret_cast<uint32_t *>(value.Real());
            }
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

} // namespace art
