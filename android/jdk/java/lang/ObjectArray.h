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

#ifndef ANDROID_JDK_JAVA_LANG_OBJECT_ARRAY_H_
#define ANDROID_JDK_JAVA_LANG_OBJECT_ARRAY_H_

#include "java/lang/Array.h"

namespace java {
namespace lang {

template <typename T = java::lang::Object>
class ObjectArray : public Array {
public:
    ObjectArray(uint32_t array) : Array(array) {}
    ObjectArray(Object& array) : Array(array) {}
    ObjectArray(Array& array) : Array(array) {}
    ObjectArray(art::mirror::Object& obj) : Array(obj) {}

    T operator[](int idx) {
        art::mirror::Array array = thiz();
        api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), idx), array);
        art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
        return object;
    }
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_OBJECT_ARRAY_H_
