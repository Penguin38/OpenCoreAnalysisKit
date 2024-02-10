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

#ifndef ANDROID_ART_RUNTIME_MIRROR_IFTABLE_H_
#define ANDROID_ART_RUNTIME_MIRROR_IFTABLE_H_

#include "runtime/mirror/object_array.h"

namespace art {
namespace mirror {

class IfTable : public ObjectArray {
public:
    IfTable(uint32_t v) : ObjectArray(v) {}
    IfTable(const ObjectArray& ref) : ObjectArray(ref) {}
    IfTable(uint32_t v, ObjectArray* ref) : ObjectArray(v, ref) {}

    inline uint32_t GetInterface(int32_t i) {
        api::MemoryRef ref(GetRawData(sizeof(uint32_t), i * kMax), this);
        return *reinterpret_cast<uint32_t *>(ref.Real());
    }
    inline int32_t Count() { return GetLength() / kMax; }

    enum {
        // Points to the interface class.
        kInterface   = 0,
        // Method pointers into the vtable, allow fast map from interface method index to concrete
        // instance method.
        kMethodArray = 1,
        kMax         = 2,
    };
};

} // namespace mirror
} // namespace art

#endif  // ANDROID_ART_RUNTIME_MIRROR_IFTABLE_H_
