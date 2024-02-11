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

#ifndef ANDROID_JDK_SUN_MISC_CLEANER_H_
#define ANDROID_JDK_SUN_MISC_CLEANER_H_

#include "java/lang/Object.h"

namespace sun {
namespace misc {

class Cleaner : public java::lang::Object {
public:
    Cleaner(uint32_t obj) : java::lang::Object(obj) {}
    Cleaner(java::lang::Object& obj) : java::lang::Object(obj) {}
    Cleaner(art::mirror::Object& obj) : java::lang::Object(obj) {}

    java::lang::Object& getReferent();
    java::lang::Object& getThunk();

private:
    java::lang::Object referent = 0x0;
    java::lang::Object thunk = 0x0;
};

} // namespace misc
} // namespace sun

#endif // ANDROID_JDK_SUN_MISC_CLEANER_H_
