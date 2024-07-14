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

#ifndef ANDROID_JDK_JAVA_LANG_THROWABLE_H_
#define ANDROID_JDK_JAVA_LANG_THROWABLE_H_

#include "java/lang/Object.h"
#include "java/lang/String.h"
#include "java/lang/ObjectArray.h"
#include "java/lang/StackTraceElement.h"

namespace java {
namespace lang {

class Throwable : public Object {
public:
    Throwable(uint32_t obj) : Object(obj) {}
    Throwable(Object& obj) : Object(obj) {}
    Throwable(art::mirror::Object& obj) : Object(obj) {}

    inline Throwable getCause() { return cause(); }
    inline String& getMessage() { return detailMessage(); }
    inline ObjectArray<StackTraceElement>& getStackTrace() { return stackTrace(); }
    static void FormatDump(const char* prefix, art::mirror::Object& obj);
private:
    DEFINE_OBJECT_FIELD_CACHE(Object, cause);
    DEFINE_OBJECT_FIELD_CACHE(String, detailMessage);
    DEFINE_OBJECT_FIELD_CACHE(ObjectArray<StackTraceElement>, stackTrace);
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_THROWABLE_H_
