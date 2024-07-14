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

#ifndef ANDROID_JDK_JAVA_LANG_THREAD_H_
#define ANDROID_JDK_JAVA_LANG_THREAD_H_

#include "java/lang/Object.h"
#include "java/lang/ThreadGroup.h"

namespace java {
namespace lang {

class Thread : public Object {
public:
    Thread(uint32_t obj) : Object(obj) {}
    Thread(Object& obj) : Object(obj) {}
    Thread(art::mirror::Object& obj) : Object(obj) {}

    inline bool getDaemon() { return GetBooleanField("daemon"); }
    inline int getPriority() { return GetIntField("priority"); }
    inline Object getTarget() { return GetObjectField("target"); }
    inline ThreadGroup& getGroup() { return group(); }
private:
    DEFINE_OBJECT_FIELD_CACHE(ThreadGroup, group);
};

} // namespace lang
} // namespace java

#endif // ANDROID_JDK_JAVA_LANG_THREAD_H_
