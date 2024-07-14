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

#ifndef ANDROID_JDK_JAVA_LANG_STACK_TRACE_ELEMENT_H_
#define ANDROID_JDK_JAVA_LANG_STACK_TRACE_ELEMENT_H_

#include "java/lang/Object.h"
#include "java/lang/String.h"

namespace java {
namespace lang {

class StackTraceElement : public Object {
public:
    StackTraceElement(uint32_t obj) : Object(obj) {}
    StackTraceElement(Object& obj) : Object(obj) {}
    StackTraceElement(art::mirror::Object& obj) : Object(obj) {}

    inline int lineNumber() { return GetIntField("lineNumber"); }

    inline String& getMethodName() { return methodName(); }
    inline String& getFileName() { return fileName(); }
    inline String& getClassName() { return declaringClass(); }
    inline int getLineNumber() { return lineNumber(); }
    inline bool isNativeMethod() { return lineNumber() == -2; }
    std::string toString();
private:
    DEFINE_OBJECT_FIELD_CACHE(String, methodName);
    DEFINE_OBJECT_FIELD_CACHE(String, fileName);
    DEFINE_OBJECT_FIELD_CACHE(String, declaringClass);
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_STACK_TRACE_ELEMENT_H_
