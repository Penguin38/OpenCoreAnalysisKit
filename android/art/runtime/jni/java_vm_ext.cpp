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

#include "api/core.h"
#include "base/mem_map.h"
#include "runtime/jni/java_vm_ext.h"
#include "common/bit.h"
#include "common/exception.h"
#include <string.h>

struct JavaVMExt_OffsetTable __JavaVMExt_offset__;
struct JavaVMExt_SizeTable __JavaVMExt_size__;

namespace art {

void JavaVMExt::Init29() {
    if (CoreApi::GetPointSize() == 64) {
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
    if (CoreApi::GetPointSize() == 64) {
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
    if (CoreApi::GetPointSize() == 64) {
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
    if (CoreApi::GetPointSize() == 64) {
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
#if defined(__PARSER_DEBUG__)
        if (weak_globals_cache.IsValid()) {
            bool found = false;
            int count = 0;
            uint64_t point_size = CoreApi::GetPointSize() / 8;
            MemMap table_mem_map_ = point_size * count + weak_globals_cache.table_mem_map();
            table_mem_map_.copyRef(weak_globals_cache);
            uint64_t endloop = RoundUp(table_mem_map_.Ptr(), 0x2000) - SIZEOF(MemMap);
            int loopcount = (endloop - table_mem_map_.Ptr()) / point_size;
            do {
                if (!table_mem_map_.IsValid())
                    break;
                try {
                    if (!strcmp("indirect ref table", table_mem_map_.GetName())) {
                        found = true;
                        break;
                    }
                } catch (InvalidAddressException e) {}
                count++;
                table_mem_map_ = point_size * count + weak_globals_cache.table_mem_map();
                table_mem_map_.copyRef(weak_globals_cache);
            } while (count < loopcount);
            if (found) {
                weak_globals_cache = table_mem_map_.Ptr() - OFFSET(IndirectReferenceTable, table_mem_map_);
                weak_globals_cache.copyRef(table_mem_map_);
            }
        }
#endif
    }
    return weak_globals_cache;
}

mirror::Object JavaVMExt::Decode(uint64_t uref) {
    IndirectRefKind kind = IndirectReferenceTable::DecodeIndirectRefKind(uref);
    if (kind == IndirectRefKind::kGlobal) {
        return DecodeGlobal(uref);
    } else if (kind == IndirectRefKind::kWeakGlobal) {
        return DecodeWeakGlobal(uref);
    }
    return 0x0;
}

mirror::Object JavaVMExt::DecodeGlobal(uint64_t uref) {
    return GetGlobalsTable().DecodeReference(IndirectReferenceTable::DecodeIndex(uref));
}

mirror::Object JavaVMExt::DecodeWeakGlobal(uint64_t uref) {
    return GetWeakGlobalsTable().DecodeReference(IndirectReferenceTable::DecodeIndex(uref));
}

} //namespace art
