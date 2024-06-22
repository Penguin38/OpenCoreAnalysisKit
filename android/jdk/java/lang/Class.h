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

#ifndef ANDROID_JDK_JAVA_LANG_CLASS_H_
#define ANDROID_JDK_JAVA_LANG_CLASS_H_

#include "java/lang/Object.h"
#include <string>
#include <vector>

namespace java {
namespace lang {

class Class : public Object {
public:
    Class(uint32_t obj) : Object(obj) {}
    Class(Object& obj) : Object(obj) {}
    Class(art::mirror::Object& obj) : Object(obj) {}

    std::string getSimpleName();
    static Class forName(const char* className);
private:
    static std::vector<uint32_t> kClassesCache;
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_CLASS_H_
