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

#ifndef ANDROID_JDK_JAVA_LANG_ARRAY_H_
#define ANDROID_JDK_JAVA_LANG_ARRAY_H_

#include "java/lang/Object.h"

namespace java {
namespace lang {

class Array : public Object {
public:
    Array(uint32_t obj) : Object(obj) {}
    Array(Object& obj) : Object(obj) {}
    Array(art::mirror::Object& obj) : Object(obj) {}

    int length();
private:
    int length_cache = -1;
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_ARRAY_H_
