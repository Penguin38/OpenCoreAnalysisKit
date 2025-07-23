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

#include "java/lang/Object.h"
#include "java/lang/Integer.h"
#include "java/lang/Class.h"
#include "runtime/mirror/iftable.h"
#include "android.h"

namespace java {
namespace lang {

bool Object::instanceof(const char* classname) {
    if (!thiz_cache.Ptr())
        return false;

    art::mirror::Class super = klass();
    do {
        if (super.PrettyDescriptor() == classname)
            return true;

        super = super.GetSuperClass();
    } while (super.Ptr());

    art::mirror::IfTable& iftable = klass().GetIfTable();
    if (!iftable.Ptr())
        return false;

    int32_t ifcount = iftable.Count();
    for (int i = 0; i < ifcount; ++i) {
        art::mirror::Class interface = iftable.GetInterface(i);
        if (interface.PrettyDescriptor() == classname)
            return true;
    }

    return false;
}

bool Object::mirror_instanceof(const char* classname) {
    if (!thiz_cache.Ptr())
        return false;

    art::mirror::Class current = Ptr();
    art::mirror::Class super = current;
    if (!thiz_cache.IsClass())
        super = klass();

    do {
        if (super.PrettyDescriptor() == classname)
            return true;

        super = super.GetSuperClass();
    } while (super.Ptr());

    art::mirror::IfTable iftable = current.GetIfTable();
    if (!thiz_cache.IsClass())
        iftable = klass().GetIfTable();

    if (!iftable.Ptr())
        return false;

    int32_t ifcount = iftable.Count();
    for (int i = 0; i < ifcount; ++i) {
        art::mirror::Class interface = iftable.GetInterface(i);
        if (interface.PrettyDescriptor() == classname)
            return true;
    }

    return false;
}

std::string Object::toString() {
    std::string sb;
    if (isNull()) {
        sb.append("null");
    } else {
        Class clazz = klass();
        sb.append(clazz.getSimpleName());
        sb.append("@");
        sb.append(Integer::toHexString(Ptr()));
    }
    return sb;
}

#define GET_INSTANCE_FIELD(TYPE, NAME) \
TYPE Object::Get##NAME##Field(const char* field, const char* classname) { \
    do { \
        TYPE value = 0x0; \
        art::mirror::Class super = klass(); \
        do { \
            if (classname && super.PrettyDescriptor() != classname) { \
                super = super.GetSuperClass(); \
                continue; \
            } \
            \
            auto callback = [&](art::ArtField& f) -> bool { \
                if (!strcmp(f.GetName(), field)) { \
                    value = f.Get##NAME(thiz_cache); \
                    return true; \
                } \
                return false; \
            }; \
            Android::ForeachInstanceField(super, callback); \
            \
            super = super.GetSuperClass(); \
        } while (super.Ptr()); \
        return value; \
    } while (0) ;\
}\

GET_INSTANCE_FIELD(uint8_t, Boolean)
GET_INSTANCE_FIELD(int8_t, Byte)
GET_INSTANCE_FIELD(uint16_t, Char)
GET_INSTANCE_FIELD(int16_t, Short)
GET_INSTANCE_FIELD(uint32_t, Object)
GET_INSTANCE_FIELD(int32_t, Int)
GET_INSTANCE_FIELD(int64_t, Long)
GET_INSTANCE_FIELD(float, Float)
GET_INSTANCE_FIELD(double, Double)

#define GET_STATIC_FIELD(TYPE, NAME) \
TYPE Object::GetStatic##NAME##Field(const char* field) { \
    do { \
        TYPE value = 0x0; \
        art::mirror::Class clazz = thiz().IsClass() ? thiz() : klass(); \
        auto callback = [&](art::ArtField& f) -> bool { \
            if (!strcmp(f.GetName(), field)) { \
                value = f.Get##NAME(clazz); \
                return true; \
            } \
            return false; \
        }; \
        Android::ForeachStaticField(clazz, callback); \
        return value; \
    } while (0) ;\
}\

GET_STATIC_FIELD(uint8_t, Boolean)
GET_STATIC_FIELD(int8_t, Byte)
GET_STATIC_FIELD(uint16_t, Char)
GET_STATIC_FIELD(int16_t, Short)
GET_STATIC_FIELD(uint32_t, Object)
GET_STATIC_FIELD(int32_t, Int)
GET_STATIC_FIELD(int64_t, Long)
GET_STATIC_FIELD(float, Float)
GET_STATIC_FIELD(double, Double)

} // namespace lang
} // namespace java
