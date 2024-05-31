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

#include "runtime/oat/oat_file.h"
#include "android.h"
#include "api/core.h"

struct OatFile_OffsetTable __OatFile_offset__;
struct OatDexFile_OffsetTable __OatDexFile_offset__;

namespace art {

void OatFile::Init() {
    if (CoreApi::GetPointSize() == 64) {
        __OatFile_offset__ = {
            .vdex_ = 32,
        };
    } else {
        __OatFile_offset__ = {
            .vdex_ = 16,
        };
    }
}

void OatDexFile::Init() {
    __OatDexFile_offset__ = {
        .oat_file_ = 0,
    };
}

VdexFile& OatFile::GetVdexFile() {
    if (!vdex_cache.Ptr()) {
        vdex_cache = vdex();
        vdex_cache.Prepare(false);
    }
    return vdex_cache;
}

uint64_t OatFile::GetVdexBegin() {
    return GetVdexFile().Begin();
}

OatFile& OatDexFile::GetOatFile() {
    if (!oat_file_cache.Ptr()) {
        oat_file_cache = oat_file();
        oat_file_cache.Prepare(false);
    }
    return oat_file_cache;
}

} // namespace art
