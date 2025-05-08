/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by oslicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_SDK_ANDROID_OS_BINDERPROXY_H_
#define ANDROID_SDK_ANDROID_OS_BINDERPROXY_H_

#include "java/lang/Object.h"

namespace android {
namespace os {

class BinderProxy : public java::lang::Object {
public:
    BinderProxy(uint32_t obj) : java::lang::Object(obj) {}
    BinderProxy(java::lang::Object& obj) : java::lang::Object(obj) {}
    BinderProxy(art::mirror::Object& obj) : java::lang::Object(obj) {}

    inline uint64_t getNativeData() { return GetLongField("mNativeData"); }
};

} // namespace os
} // namespace android

#endif // ANDROID_SDK_ANDROID_OS_BINDERPROXY_H_
