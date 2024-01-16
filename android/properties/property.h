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

#ifndef ANDROID_PROPERTIES_PROPERTY_H_
#define ANDROID_PROPERTIES_PROPERTY_H_

#include "properties/prop_info.h"
#include <functional>

namespace android {
class Property {
public:
    static void Init();
    static const char* Get(const char *name);
    static const char* Get(const char *name, const char* def);
    static int64_t GetInt64(const char *name);
    static int64_t GetInt64(const char *name, int64_t def);
    static int32_t GetInt32(const char *name);
    static int32_t GetInt32(const char *name, int32_t def);
    static void Foreach(std::function<void (PropInfo& info)> callback);
};
} // android

#endif // ANDROID_PROPERTIES_PROPERTY_H_
