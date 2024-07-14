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

#include "java/lang/StackTraceElement.h"

namespace java {
namespace lang {

std::string StackTraceElement::toString() {
    std::string result;
    String& declaringClass = getClassName();
    String& methodName = getMethodName();
    String& fileName = getFileName();
    int lineNumber = getLineNumber();

    result.append(declaringClass.toString()).append(".").append(methodName.toString());
    if (isNativeMethod()) {
        result.append("(Native Method)");
    } else if (!fileName.isNull()) {
        if (lineNumber >= 0) {
            result.append("(").append(fileName.toString()).append(":").append(std::to_string(lineNumber)).append(")");
        } else {
            result.append("(").append(fileName.toString()).append(")");
        }
    } else {
        if (lineNumber >= 0) {
            result.append("(Unknown Source:").append(std::to_string(lineNumber)).append(")");
        } else {
            result.append("(Unknown Source)");
        }
    }
    return result;
}

} // namespace lang
} // namespace java
