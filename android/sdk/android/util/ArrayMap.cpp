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

#include "android/util/ArrayMap.h"
#include "runtime/mirror/array.h"

namespace android {
namespace util {

java::lang::Object ArrayMap::keyAt(int idx) {
    art::mirror::Array array = GetObjectField("mArray");
    api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), idx << 1), array);
    art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
    return object;
}

java::lang::Object ArrayMap::valueAt(int idx) {
    art::mirror::Array array = GetObjectField("mArray");
    api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), (idx << 1) + 1), array);
    art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
    return object;
}

java::lang::Object ArrayMap::get(java::lang::Object& key) {
    for (int idx = 0; idx < size(); ++idx) {
        if (keyAt(idx) == key) {
            return valueAt(idx);
        }
    }
    return 0x0;
}

} // namespace util
} // namespace android
