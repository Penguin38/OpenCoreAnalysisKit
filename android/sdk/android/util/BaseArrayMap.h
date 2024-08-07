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

#ifndef ANDROID_SDK_ANDROID_UTIL_BASE_ARRAYMAP_H_
#define ANDROID_SDK_ANDROID_UTIL_BASE_ARRAYMAP_H_

#include "java/lang/Object.h"
#include "java/lang/ObjectArray.h"

namespace android {
namespace util {

class BaseArrayMap : public java::lang::Object {
public:
    BaseArrayMap(uint32_t obj) : java::lang::Object(obj) {}
    BaseArrayMap(java::lang::Object& obj) : java::lang::Object(obj) {}
    BaseArrayMap(art::mirror::Object& obj) : java::lang::Object(obj) {}

    inline int size() { return GetIntField("mSize"); }
    inline java::lang::ObjectArray<java::lang::Object>& getArray() { return mArray(); }
private:
    DEFINE_OBJECT_FIELD_CACHE(java::lang::ObjectArray<java::lang::Object>, mArray);
};

} // namespace util
} // namespace android

#endif // ANDROID_SDK_ANDROID_UTIL_BASE_ARRAYMAP_H_
