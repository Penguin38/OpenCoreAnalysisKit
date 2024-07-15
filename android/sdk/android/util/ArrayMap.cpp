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
#include "java/lang/String.h"
#include "android/util/ArrayMap.h"

namespace android {
namespace util {

template <typename K, typename V>
std::string ArrayMap<K, V>::toString() {
    std::string sb;
    int mSize = size();

    sb.append("{");
    for (int idx = 0; idx < mSize; ++idx) {
        K key = keyAt(idx);
        V value = valueAt(idx);

        if (idx > 0)
            sb.append(", ");

        if (key.instanceof("java.lang.String")) {
            java::lang::String str = key;
            sb.append(str.toString());
        } else {
            sb.append(key.toString());
        }

        sb.append("=");

        if (value.instanceof("java.lang.String")) {
            java::lang::String str = value;
            sb.append(str.toString());
        } else {
            sb.append(value.toString());
        }
    }
    sb.append("}");
    return sb;
}

template<>
void ArrayMap<>::FormatDump(const char* prefix, art::mirror::Object& obj) {
    ArrayMap<> array = obj;
    int mSize = array.size();
    for (int idx = 0; idx < mSize; ++idx) {
        std::string sb;
        java::lang::Object key = array.keyAt(idx);
        java::lang::Object value = array.valueAt(idx);
        sb.append("{");
        if (key.instanceof("java.lang.String")) {
            java::lang::String str = key;
            sb.append(str.toString());
        } else {
            sb.append(key.toString());
        }
        sb.append(", ");
        if (value.instanceof("java.lang.String")) {
            java::lang::String str = value;
            sb.append(str.toString());
        } else {
            sb.append(value.toString());
        }
        sb.append("}");
        LOGI("%s[%d] %s\n", prefix, idx, sb.c_str());
    }
}

} // namespace util
} // namespace android
