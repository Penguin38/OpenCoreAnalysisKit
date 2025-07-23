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

#ifndef ANDROID_JDK_JAVA_LANG_OBJECT_H_
#define ANDROID_JDK_JAVA_LANG_OBJECT_H_

#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"

namespace java {
namespace lang {

class Object {
public:
    Object(uint32_t obj) { thiz_cache = obj; }
    Object(Object& obj) { thiz_cache = obj.thiz_cache; }
    Object(art::mirror::Object& obj) { thiz_cache = obj; }

    bool instanceof(const char* classname);
    bool mirror_instanceof(const char* classname);

    inline bool isNull() { return !thiz_cache.Ptr(); }
    inline uint32_t Ptr() { return thiz_cache.Ptr(); }
    inline art::mirror::Class& klass() {
        if (!klass_cache.Ptr()) {
            klass_cache = thiz_cache.GetClass();
        }
        return klass_cache;
    }
    inline bool IsValid() { return !isNull() && thiz_cache.IsValid(); }

    uint8_t GetBooleanField(const char* field) { return GetBooleanField(field, nullptr); }
    uint8_t GetBooleanField(const char* field, const char* classname);
    int8_t GetByteField(const char* field) { return GetByteField(field, nullptr); }
    int8_t GetByteField(const char* field, const char* classname);
    uint16_t GetCharField(const char* field) { return GetCharField(field, nullptr); }
    uint16_t GetCharField(const char* field, const char* classname);
    int16_t GetShortField(const char* field) { return GetShortField(field, nullptr); }
    int16_t GetShortField(const char* field, const char* classname);
    uint32_t GetObjectField(const char* field) { return GetObjectField(field, nullptr); }
    uint32_t GetObjectField(const char* field, const char* classname);
    int32_t GetIntField(const char* field) { return GetIntField(field, nullptr); }
    int32_t GetIntField(const char* field, const char* classname);
    int64_t GetLongField(const char* field) { return GetLongField(field, nullptr); }
    int64_t GetLongField(const char* field, const char* classname);
    float GetFloatField(const char* field) { return GetFloatField(field, nullptr); }
    float GetFloatField(const char* field, const char* classname);
    double GetDoubleField(const char* field) { return GetDoubleField(field, nullptr); }
    double GetDoubleField(const char* field, const char* classname);

    uint8_t GetStaticBooleanField(const char* field);
    int8_t GetStaticByteField(const char* field);
    uint16_t GetStaticCharField(const char* field);
    int16_t GetStaticShortField(const char* field);
    uint32_t GetStaticObjectField(const char* field);
    int32_t GetStaticIntField(const char* field);
    int64_t GetStaticLongField(const char* field);
    float GetStaticFloatField(const char* field);
    double GetStaticDoubleField(const char* field);

    inline art::mirror::Object& thiz() { return thiz_cache; }

    inline bool operator==(Object& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(Object& ref) { return Ptr() != ref.Ptr(); }

    std::string toString();
private:
    // quick memoryref cache;
    art::mirror::Object thiz_cache = 0;
    art::mirror::Class klass_cache = 0;
};

} // namespace lang
} // namespace java

#define DEFINE_OBJECT_FIELD_CACHE(T, NAME) \
T NAME##_cache = 0x0; \
inline T& NAME() { \
    do { \
        if (NAME##_cache.isNull()) {\
            NAME##_cache = GetObjectField(#NAME); \
            NAME##_cache.thiz().copyRef(thiz()); \
        } \
        return NAME##_cache;\
    } while (0); \
} \

#define DEFINE_OBJECT_STATIC_FIELD_CACHE(T, NAME) \
T NAME##_cache = 0x0; \
inline T& NAME() { \
    do { \
        if (NAME##_cache.isNull()) {\
            NAME##_cache = GetStaticObjectField(#NAME); \
            NAME##_cache.thiz().copyRef(thiz()); \
        } \
        return NAME##_cache;\
    } while (0); \
} \

#endif // ANDROID_JDK_JAVA_LANG_OBJECT_H_
