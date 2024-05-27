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
    int32_t ifcount = iftable.Count();
    for (int i = 0; i < ifcount; ++i) {
        art::mirror::Class interface = iftable.GetInterface(i);
        if (interface.PrettyDescriptor() == classname)
            return true;
    }

    return false;
}

uint8_t Object::GetBooleanField(const char* field, const char* classname) {
    uint8_t value = 0x0;
    art::mirror::Class super = klass();
    do {
        if (classname && super.PrettyDescriptor() != classname) {
            super = super.GetSuperClass();
            continue;
        }

        auto callback = [&](art::ArtField& f) -> bool {
            if (!strcmp(f.GetName(), field)) {
                value = f.GetBoolean(thiz_cache);
                return true;
            }
            return false;
        };
        Android::ForeachInstanceField(super, callback);

        super = super.GetSuperClass();
    } while (super.Ptr());
    return value;
}

uint32_t Object::GetObjectField(const char* field, const char* classname) {
    uint32_t obj = 0x0;
    art::mirror::Class super = klass();
    do {
        if (classname && super.PrettyDescriptor() != classname) {
            super = super.GetSuperClass();
            continue;
        }

        auto callback = [&](art::ArtField& f) -> bool {
            if (!strcmp(f.GetName(), field)) {
                obj = f.GetObj(thiz_cache);
                return true;
            }
            return false;
        };
        Android::ForeachInstanceField(super, callback);

        super = super.GetSuperClass();
    } while (super.Ptr());
    return obj;
}

int32_t Object::GetIntField(const char* field, const char* classname) {
    int32_t value = 0x0;
    art::mirror::Class super = klass();
    do {
        if (classname && super.PrettyDescriptor() != classname) {
            super = super.GetSuperClass();
            continue;
        }

        auto callback = [&](art::ArtField& f) -> bool {
            if (!strcmp(f.GetName(), field)) {
                value = f.GetInt(thiz_cache);
                return true;
            }
            return false;
        };
        Android::ForeachInstanceField(super, callback);

        super = super.GetSuperClass();
    } while (super.Ptr());
    return value;
}

int64_t Object::GetLongField(const char* field, const char* classname) {
    int64_t value = 0x0;
    art::mirror::Class super = klass();
    do {
        if (classname && super.PrettyDescriptor() != classname) {
            super = super.GetSuperClass();
            continue;
        }

        auto callback = [&](art::ArtField& f) -> bool {
            if (!strcmp(f.GetName(), field)) {
                value = f.GetLong(thiz_cache);
                return true;
            }
            return false;
        };
        Android::ForeachInstanceField(super, callback);

        super = super.GetSuperClass();
    } while (super.Ptr());
    return value;
}

} // namespace lang
} // namespace java
