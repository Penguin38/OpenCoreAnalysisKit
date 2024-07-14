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

#ifndef ANDROID_SDK_ANDROID_APP_CONTEXTIMPL_H_
#define ANDROID_SDK_ANDROID_APP_CONTEXTIMPL_H_

#include "java/lang/Object.h"

namespace android {
namespace app {

class ContextImpl : public java::lang::Object {
public:
    ContextImpl(uint32_t obj) : java::lang::Object(obj) {}
    ContextImpl(java::lang::Object& obj) : java::lang::Object(obj) {}
    ContextImpl(art::mirror::Object& obj) : java::lang::Object(obj) {}

    inline java::lang::Object& getServiceCache() { return mServiceCache(); }
    java::lang::Object getSystemService(const char* serviceName);
private:
    DEFINE_OBJECT_FIELD_CACHE(java::lang::Object, mServiceCache);
};

} // namespace app
} // namespace android

#endif // ANDROID_SDK_ANDROID_APP_CONTEXTIMPL_H_
