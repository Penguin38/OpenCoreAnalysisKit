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
#include "dex/dex_file.h"
#include "base/bit_vector.h"
#include "android.h"
#include "api/core.h"

struct OatFile_OffsetTable __OatFile_offset__;
struct OatDexFile_OffsetTable __OatDexFile_offset__;

namespace art {

void OatFile::Init() {
    Android::RegisterSdkListener(Android::M, art::OatFile::Init23);
    Android::RegisterSdkListener(Android::O, art::OatFile::Init26);
}

void OatDexFile::Init() {
    Android::RegisterSdkListener(Android::M, art::OatDexFile::Init23);
    Android::RegisterSdkListener(Android::N, art::OatDexFile::Init24);
    Android::RegisterSdkListener(Android::P, art::OatDexFile::Init28);
    Android::RegisterSdkListener(Android::S, art::OatDexFile::Init31);
    Android::RegisterSdkListener(Android::V, art::OatDexFile::Init35);
}

void OatFile::Init23() {
    if (CoreApi::Bits() == 64) {
        __OatFile_offset__ = {
            .begin_ = 24,
            .is_executable_ = 64,
        };
    } else {
        __OatFile_offset__ = {
            .begin_ = 12,
            .is_executable_ = 28,
        };
    }
}

void OatFile::Init26() {
    if (CoreApi::Bits() == 64) {
        __OatFile_offset__ = {
            .vdex_ = 32,
            .begin_ = 40,
            .is_executable_ = 104,
        };
    } else {
        __OatFile_offset__ = {
            .vdex_ = 16,
            .begin_ = 20,
            .is_executable_ = 52,
        };
    }
}

void OatDexFile::Init23() {
    if (CoreApi::Bits() == 64) {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 72,
        };
    } else {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 36,
        };
    }
}

void OatDexFile::Init24() {
    if (CoreApi::Bits() == 64) {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 80,
        };
    } else {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 40,
        };
    }
}

void OatDexFile::Init28() {
    if (CoreApi::Bits() == 64) {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 104,
        };
    } else {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 52,
        };
    }
}

void OatDexFile::Init31() {
    if (CoreApi::Bits() == 64) {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 120,
        };
    } else {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 60,
        };
    }
}

void OatDexFile::Init35() {
    if (CoreApi::Bits() == 64) {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 168,
        };
    } else {
        __OatDexFile_offset__ = {
            .oat_file_ = 0,
            .oat_class_offsets_pointer_ = 96,
        };
    }
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

OatFile::OatClass OatFile::FindOatClass(DexFile& dex_file, uint16_t class_def_idx, bool* found) {
    OatDexFile& oat_dex_file = dex_file.GetOatDexFile();
    if (!oat_dex_file.Ptr() || !oat_dex_file.GetOatFile().Ptr()) {
        *found = false;
        return OatFile::OatClass::Invalid();
    }
    *found = true;
    return oat_dex_file.GetOatClass(class_def_idx);
}

OatFile::OatMethod OatFile::OatClass::GetOatMethod(uint32_t method_index) {
    api::MemoryRef method_offsets(GetOatMethodOffsets(method_index));

    if (!method_offsets.Ptr()) {
        return OatMethod(0, 0);
    }

    OatFile oat_file(oat_file_);
    if (oat_file.is_executable()) {
        return OatMethod(oat_file.begin(), method_offsets.value32Of());
    }
    return OatMethod(oat_file.begin(), 0);
}

api::MemoryRef OatFile::OatClass::GetOatMethodOffsets(uint32_t method_index) {
    if (!methods_pointer_.Ptr()) {
        return 0x0;
    }

    uint32_t methods_pointer_index;
    if (!bitmap_.Ptr()) {
        methods_pointer_index = method_index;
    } else {
        if (!BitVector::IsBitSet(bitmap_, method_index)) {
            return 0x0;
        }
        uint32_t num_set_bits = BitVector::NumSetBits(bitmap_, method_index);
        methods_pointer_index = num_set_bits;
    }

    return methods_pointer_.Ptr() + methods_pointer_index * sizeof(uint32_t);
}

OatFile& OatDexFile::GetOatFile() {
    if (!oat_file_cache.Ptr()) {
        oat_file_cache = oat_file();
        oat_file_cache.Prepare(false);
    }
    return oat_file_cache;
}

OatFile::OatClass OatDexFile::GetOatClass(uint16_t class_def_index) {
    if (IsBackedByVdexOnly()) {
        return OatFile::OatClass::Invalid();
    }

    api::MemoryRef oat_class_offsets_pointer_ = oat_class_offsets_pointer();
    uint32_t oat_class_offset = oat_class_offsets_pointer_.value32Of(class_def_index * sizeof(uint32_t));

    OatFile& oat_file_ = GetOatFile();
    api::MemoryRef current_pointer = oat_file_.begin() + oat_class_offset;

    uint16_t status_value = current_pointer.value16Of();
    current_pointer.MovePtr(sizeof(uint16_t));

    uint16_t type_value = current_pointer.value16Of();
    current_pointer.MovePtr(sizeof(uint16_t));

    ClassStatus status = static_cast<ClassStatus>(status_value);
    OatClassType type = static_cast<OatClassType>(type_value);

    uint32_t num_methods = 0;
    uint32_t bitmap_size = 0;
    uint64_t bitmap_pointer = 0x0;
    uint64_t methods_pointer = 0x0;

    if (type != OatClassType::kNoneCompiled) {
        if (Android::Sdk() >= Android::S) {
            num_methods = current_pointer.value32Of();
            current_pointer.MovePtr(sizeof(uint32_t));
            if (type == OatClassType::kSomeCompiled) {
                uint32_t bitmap_size = BitVector::BitsToWords(num_methods) * BitVector::kWordBytes;
                bitmap_pointer = current_pointer.Ptr();
                current_pointer.MovePtr(bitmap_size);
            }
            methods_pointer = current_pointer.Ptr();
        } else {
            if (type == OatClassType::kSomeCompiled) {
                bitmap_size = current_pointer.value32Of();
                bitmap_pointer = current_pointer.Ptr() + sizeof(bitmap_size);
                methods_pointer = bitmap_pointer + bitmap_size;
            } else {
                methods_pointer = current_pointer.Ptr();
            }
            num_methods = bitmap_size;
        }
    }

    return OatFile::OatClass(oat_file_.Ptr(), status, type, num_methods, bitmap_pointer, methods_pointer);
}

} // namespace art
