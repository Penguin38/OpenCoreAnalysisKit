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

#include "java/lang/ThreadGroup.h"

namespace java {
namespace lang {

String& ThreadGroup::getName() {
    if (name.isNull()) {
        name = GetObjectField("name");
    }
    return name;
}

std::string ThreadGroup::Name() {
    return getName().toString();
}

} // namespace lang
} // namespace java
