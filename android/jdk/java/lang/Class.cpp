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

#include "java/lang/Class.h"
#include "runtime/mirror/string.h"
#include "android.h"

namespace java {
namespace lang {

std::vector<uint32_t> Class::kClassesCache;

std::string Class::getSimpleName() {
    art::mirror::Class clazz = thiz();
    return clazz.PrettyDescriptor();
}

Class Class::forName(const char* className) {
    for (const auto& cache : kClassesCache) {
        Class clazz = cache;
        if (clazz.getSimpleName() == className) {
            return clazz;
        }
    }

    Class thiz_clazz = 0x0;
    auto callback = [&](art::mirror::Object& object) -> bool {
        if (!object.IsClass())
            return false;

        Class clazz = object;
        if (clazz.getSimpleName() == className) {
            thiz_clazz = clazz;
            kClassesCache.push_back(thiz_clazz.Ptr());
            return true;
        }
        return false;
    };
    Android::ForeachObjects(callback);
    return thiz_clazz;
}

} // namespace lang
} // namespace java
