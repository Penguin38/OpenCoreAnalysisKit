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

#include "runtime/vdex_file.h"
#include "android.h"

struct VdexFile_OffsetTable __VdexFile_offset__;

namespace art {

void VdexFile::Init() {
    __VdexFile_offset__ = {
        .mmap_ = 0,
    };
}

MemMap& VdexFile::GetMap() {
    if (!mmap_cache.Ptr()) {
        if (Android::Sdk() >= Android::Q) {
            mmap_cache = mmap();
        } else {
            mmap_cache = mmap_lv29();
        }
        mmap_cache.Prepare(false);
    }
    return mmap_cache;
}

const char* VdexFile::GetName() {
    return GetMap().GetName();
}

uint64_t VdexFile::Begin() {
    return GetMap().begin();
}

} //namespace art
