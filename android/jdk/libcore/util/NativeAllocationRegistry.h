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

#ifndef ANDROID_JDK_LIBCORE_UTIL_NATIVEALLOCATIONREGISTRY_H_
#define ANDROID_JDK_LIBCORE_UTIL_NATIVEALLOCATIONREGISTRY_H_

#include "java/lang/Object.h"

namespace libcore {
namespace util {

class NativeAllocationRegistry : public java::lang::Object {
public:
    NativeAllocationRegistry(uint32_t obj) : java::lang::Object(obj) {}
    NativeAllocationRegistry(java::lang::Object& obj) : java::lang::Object(obj) {}
    NativeAllocationRegistry(art::mirror::Object& obj) : java::lang::Object(obj) {}

    inline int64_t getSize() { return GetLongField("size"); }

    class CleanerThunk : public java::lang::Object {
    public:
        CleanerThunk(uint32_t obj) : java::lang::Object(obj) {}
        CleanerThunk(java::lang::Object& obj) : java::lang::Object(obj) {}
        CleanerThunk(art::mirror::Object& obj) : java::lang::Object(obj) {}

        java::lang::Object& getRegistry();
    private:
        java::lang::Object this0 = 0x0;
    };
};

} // namespace util
} // namespace libcore

#endif // ANDROID_JDK_LIBCORE_UTIL_NATIVEALLOCATIONREGISTRY_H_
