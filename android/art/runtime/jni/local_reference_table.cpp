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

#include "logger/log.h"
#include "api/core.h"
#include "android.h"
#include "cxx/vector.h"
#include "runtime/indirect_reference_table.h"
#include "runtime/jni/local_reference_table.h"

struct LocalReferenceTable_OffsetTable __LocalReferenceTable_offset__;
struct LrtEntry_OffsetTable __LrtEntry_offset__;
struct LrtEntry_SizeTable __LrtEntry_size__;

namespace art {
namespace jni {

void LocalReferenceTable::Init() {
    Android::RegisterSdkListener(Android::U, art::jni::LocalReferenceTable::Init34);
    Android::RegisterSdkListener(Android::V, art::jni::LocalReferenceTable::Init35);

    art::jni::LrtEntry::Init();
}

void LocalReferenceTable::Init34() {
    if (CoreApi::Bits() == 64) {
        __LocalReferenceTable_offset__ = {
            .segment_state_ = 0,
            .tables_ = 24,
        };
    } else {
        __LocalReferenceTable_offset__ = {
            .segment_state_ = 0,
            .tables_ = 16,
        };
    }
}

void LocalReferenceTable::Init35() {
    if (CoreApi::Bits() == 64) {
        __LocalReferenceTable_offset__ = {
            .segment_state_ = 4,
            .tables_ = 24,
        };
    } else {
        __LocalReferenceTable_offset__ = {
            .segment_state_ = 4,
            .tables_ = 20,
        };
    }
}

void LrtEntry::Init() {
    __LrtEntry_offset__.root_ = 0;
    __LrtEntry_size__.THIS = 4;

    kSmallLrtEntries = kInitialLrtBytes / SIZEOF(LrtEntry);
}

mirror::Object LocalReferenceTable::DecodeReference(uint64_t iref) {
    LrtEntry entry = iref & ~static_cast<uint64_t>(kKindMask);
    cxx::vector tables_ = tables();
    for (const auto& value : tables_) {
        api::MemoryRef ref = value;
        LrtEntry local = ref.valueOf();
        if (!local.IsValid() || !local.Block()->virtualContains(entry.Ptr()))
            continue;

        mirror::Object object = entry.root();
        return object;
    }
    return 0x0;
}

void LocalReferenceTable::Walk(std::function<bool (mirror::Object& object)> fn) {
    auto callback = [&](art::mirror::Object& object, uint64_t iref) -> bool {
        fn(object);
        return false;
    };
    Walk(callback);
}

void LocalReferenceTable::Walk(std::function<bool (mirror::Object& object, uint64_t iref)> fn) {
    cxx::vector tables_ = tables();
    tables_.SetEntrySize(CoreApi::GetPointSize());
    for (int i = 0; i < tables_.size(); ++i) {
        api::MemoryRef ref = tables_[i];
        LrtEntry entry = ref.valueOf();
        for (int pos = 0; pos < kSmallLrtEntries; pos += SIZEOF(LrtEntry)) {
            mirror::Object object = entry.value32Of(pos);
            if (object.Ptr() && object.IsValid()) {
                fn(object, (entry.Ptr() + pos) | IndirectRefKind::kLocal);
            }
        }
    }
}

} // namespace jni
} // namespace art
