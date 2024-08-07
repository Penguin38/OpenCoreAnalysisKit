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
#include "runtime/mirror/string.h"

namespace java {
namespace lang {

std::string String::toString() {
    art::mirror::String mirror = thiz();
    if (mirror.Ptr()) {
        return mirror.ToModifiedUtf8();
    } else {
        return "null";
    }
}

void String::FormatDump(const char* prefix, art::mirror::Object& obj) {
    String value = obj;
    LOGI("%s%s\n", prefix, value.toString().c_str());
}

} // namespace lang
} // namespace java
