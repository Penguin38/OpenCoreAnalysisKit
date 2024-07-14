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

#ifndef ANDROID_SDK_ANDROID_UTIL_ARRAYMAP_H_
#define ANDROID_SDK_ANDROID_UTIL_ARRAYMAP_H_

#include "logger/log.h"
#include "android/util/BaseArrayMap.h"

namespace android {
namespace util {

template <typename K, typename V>
class ArrayMap : public BaseArrayMap {
public:
    ArrayMap(uint32_t map) : BaseArrayMap(map) {}
    ArrayMap(java::lang::Object& obj) : BaseArrayMap(obj) {}
    ArrayMap(BaseArrayMap& map) : BaseArrayMap(map) {}
    ArrayMap(art::mirror::Object& obj) : BaseArrayMap(obj) {}

    K keyAt(int idx) {
        java::lang::ObjectArray<java::lang::Object>& mArray = getArray();
        java::lang::Object key = mArray[idx << 1];
        return key;
    }

    V valueAt(int idx) {
        java::lang::ObjectArray<java::lang::Object>& mArray = getArray();
        java::lang::Object value = mArray[(idx << 1) + 1];
        return value;
    }

    V get(K& key) {
        for (int idx = 0; idx < size(); ++idx) {
            if (keyAt(idx).thiz() == key.thiz()) {
                return valueAt(idx);
            }
        }
        return 0x0;
    }

    std::string toString();
    static void FormatDump(const char* prefix, art::mirror::Object& obj);
};

} // namespace util
} // namespace android

#endif // ANDROID_SDK_ANDROID_UTIL_ARRAYMAP_H_
