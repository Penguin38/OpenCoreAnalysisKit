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

#ifndef ANDROID_JDK_JAVA_LANG_INTEGER_H_
#define ANDROID_JDK_JAVA_LANG_INTEGER_H_

#include "java/lang/Object.h"
#include "base/utils.h"

namespace java {
namespace lang {

class Integer : public Object {
public:
    Integer(uint32_t obj) : Object(obj) {}
    Integer(Object& obj) : Object(obj) {}
    Integer(art::mirror::Object& obj) : Object(obj) {}

    inline static std::string toHexString(uint32_t value) {
        return Utils::ToHex(value);
    }
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_INTEGER_H_
