/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef PARSER_COMMAND_ANDROID_VERIFY_H_
#define PARSER_COMMAND_ANDROID_VERIFY_H_

#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/array.h"
#include "runtime/art_method.h"
#include <unordered_map>

class JavaVerify {
public:
    constexpr static int CHECK_FULL_BAD_OBJECT = 1 << 0;
    constexpr static int CHECK_FULL_CONFLICT_METHOD = 1 << 1;
    constexpr static int CHECK_FULL_REUSE_DEX_PC_PTR = 1 << 2;

    void init(int options, int flag);
    bool isEnabled() { return (options > 0); }
    void verify(art::mirror::Object& thiz);
    static void VerifyClassObject(art::mirror::Class& clazz);
    static void VerifyArrayObject(art::mirror::Array& array);
    static void VerifyInstanceObject(art::mirror::Object& object);
    static void VerifyConflictMethod(art::mirror::Class& clazz);
    void VerifyReuseDexPcMethod(art::mirror::Class& clazz);
    void verifyMethods();
    static uint64_t FindSuperMethodToCall(art::ArtMethod& method, std::string name);
private:
    int options;
    int flags;
    std::unordered_map<uint64_t, uint64_t> methods;
};

#endif // PARSER_COMMAND_ANDROID_VERIFY_H_
