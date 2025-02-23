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

#include "api/core.h"
#include "android.h"
#include "base/mem_map.h"
#include "runtime/jni/java_vm_ext.h"
#include "common/bit.h"
#include "common/exception.h"
#include <string.h>

struct JavaVMExt_OffsetTable __JavaVMExt_offset__;
struct JavaVMExt_SizeTable __JavaVMExt_size__;

namespace art {

void JavaVMExt::Init() {
    Android::RegisterSdkListener(Android::M, art::JavaVMExt::Init23);
    Android::RegisterSdkListener(Android::N, art::JavaVMExt::Init24);
    Android::RegisterSdkListener(Android::O, art::JavaVMExt::Init26);
    Android::RegisterSdkListener(Android::Q, art::JavaVMExt::Init29);
    Android::RegisterSdkListener(Android::S, art::JavaVMExt::Init31);
    Android::RegisterSdkListener(Android::T, art::JavaVMExt::Init33);
    Android::RegisterSdkListener(Android::U, art::JavaVMExt::Init34);
}

void JavaVMExt::Init23() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 112,
            .weak_globals_ = 224,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 72,
            .weak_globals_ = 144,
        };

        /* 6.0.1 */
        if (Android::Patch() >= 1) {
            __JavaVMExt_offset__.weak_globals_ = 152;
        }
    }
}

void JavaVMExt::Init24() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 112,
            .weak_globals_ = 224,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 72,
            .weak_globals_ = 144,
        };
    }
}

void JavaVMExt::Init26() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 64,
            .weak_globals_ = 136,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 32,
            .weak_globals_ = 72,
        };
    }
}

void JavaVMExt::Init29() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 64,
            .weak_globals_ = 200,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 32,
            .weak_globals_ = 108,
        };
    }
}

void JavaVMExt::Init31() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 64,
            .weak_globals_ = 200,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 32,
            .weak_globals_ = 108,
        };
    }
}

void JavaVMExt::Init33() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 64,
            .weak_globals_ = 200,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 32,
            .weak_globals_ = 108,
        };
    }
}

void JavaVMExt::Init34() {
    if (CoreApi::Bits() == 64) {
        __JavaVMExt_offset__ = {
            .globals_ = 64,
            .weak_globals_ = 192,
        };
    } else {
        __JavaVMExt_offset__ = {
            .globals_ = 32,
            .weak_globals_ = 100,
        };
    }
}

IndirectReferenceTable& JavaVMExt::GetGlobalsTable() {
    if (!globals_cache.Ptr()) {
        globals_cache = globals();
        globals_cache.copyRef(this);
        globals_cache.Prepare(false);
    }
    return globals_cache;
}

IndirectReferenceTable& JavaVMExt::GetWeakGlobalsTable() {
    if (!weak_globals_cache.Ptr()) {
        weak_globals_cache = weak_globals();
        weak_globals_cache.copyRef(this);
        weak_globals_cache.Prepare(false);
#if defined(__ART_JVM_WEAK_GLOBALS_PARSER__)
        if (Android::Sdk() >= Android::P) {
            if (weak_globals_cache.IsValid()) {
                bool found = false;
                int count = 0;
                uint64_t point_size = CoreApi::GetPointSize();
                MemMap table_mem_map_ = point_size * count + weak_globals_cache.table_mem_map();
                table_mem_map_.copyRef(weak_globals_cache);

                /* near memory match */
                int loopcount = 200;
                do {
                    if (!table_mem_map_.IsValid())
                        break;
                    try {
                        if (!strcmp("indirect ref table", table_mem_map_.GetName())) {
                            found = true;
                            break;
                        }
                    } catch (InvalidAddressException& e) {}
                    count++;
                    table_mem_map_ = point_size * count + weak_globals_cache.table_mem_map();
                    table_mem_map_.copyRef(weak_globals_cache);
                } while (count < loopcount);

                if (found && count) {
                    weak_globals_cache = table_mem_map_.Ptr() - OFFSET(IndirectReferenceTable, table_mem_map_);
                    weak_globals_cache.copyRef(table_mem_map_);
                    LOGD(">>> 'weak_globals_' = 0x%" PRIx64 "\n", weak_globals_cache.Ptr());
                }
            }
        }
#endif
    }
    return weak_globals_cache;
}

mirror::Object JavaVMExt::Decode(uint64_t uref) {
    try {
        IndirectRefKind kind = IndirectReferenceTable::DecodeIndirectRefKind(uref);
        if (kind == IndirectRefKind::kGlobal) {
            return DecodeGlobal(uref);
        } else if (kind == IndirectRefKind::kWeakGlobal) {
            return DecodeWeakGlobal(uref);
        }
    } catch(InvalidAddressException& e) {}
    return 0x0;
}

mirror::Object JavaVMExt::DecodeGlobal(uint64_t uref) {
    return GetGlobalsTable().DecodeReference(IndirectReferenceTable::DecodeIndex(uref));
}

mirror::Object JavaVMExt::DecodeWeakGlobal(uint64_t uref) {
    return GetWeakGlobalsTable().DecodeReference(IndirectReferenceTable::DecodeIndex(uref));
}

} //namespace art
