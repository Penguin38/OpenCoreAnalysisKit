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
#include "java/lang/Throwable.h"
#include "java/lang/Class.h"

namespace java {
namespace lang {

#if 0
Object& Throwable::cause() {
    if (cause_cache.isNull()) {
        cause_cache = GetObjectField("cause");
        cause_cache.thiz().copyRef(thiz());
    }
    return cause_cache;
}

String& Throwable::detailMessage() {
    if (detailMessage_cache.isNull()) {
        detailMessage_cache = GetObjectField("detailMessage");
        detailMessage_cache.thiz().copyRef(thiz());
    }
    return detailMessage_cache;
}

ObjectArray<StackTraceElement>& Throwable::stackTrace() {
    if (stackTrace_cache.isNull()) {
        stackTrace_cache = GetObjectField("stackTrace");
        stackTrace_cache.thiz().copyRef(thiz());
    }
    return stackTrace_cache;
}
#endif

void Throwable::FormatDump(const char* prefix, art::mirror::Object& obj) {
    Throwable throwable = obj;
    String& message = throwable.getMessage();
    Class clazz = throwable.klass();
    LOGI("%sThrowable %s\n", prefix, clazz.getSimpleName().c_str());
    if (!message.isNull())
        LOGI("%s%s\n", prefix, message.toString().c_str());

    ObjectArray<StackTraceElement>& stackTrace = throwable.getStackTrace();
    if (!stackTrace.isNull()) {
        for (int i = 0; i < stackTrace.length(); i++) {
            StackTraceElement element = stackTrace[i];
            LOGI("    %s%s\n", prefix, element.toString().c_str());
        }
    }

    Throwable cause = throwable.getCause();
    if (cause.thiz() != throwable.thiz()) {
        LOGI("%sCaused by:\n", prefix);
        FormatDump(prefix, cause.thiz());
    }
}

} // namespace lang
} // namespace java
