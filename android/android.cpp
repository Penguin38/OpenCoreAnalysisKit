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

#include "logger/log.h"
#include "android.h"
#include "properties/property.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/string.h"
#include "runtime/mirror/array.h"
#include "runtime/mirror/dex_cache.h"
#include "dex/dex_file.h"
#include "dex/dex_file_structs.h"

Android* Android::INSTANCE = nullptr;

void Android::Init() {
    if (!INSTANCE) {
        INSTANCE = new Android();
        art::mirror::Object::Init();
        art::mirror::Class::Init();
        art::mirror::String::Init();
        art::mirror::Array::Init();
        art::mirror::DexCache::Init();
        art::DexFile::Init();
        art::dex::TypeId::Init();
        art::dex::StringId::Init();
        art::dex::FieldId::Init();
        art::dex::MethodId::Init();
        art::dex::ProtoId::Init();
        art::dex::TypeList::Init();
        art::dex::TypeItem::Init();
        INSTANCE->init();
    }
}

void Android::Clean() {
    if (INSTANCE) {
        delete INSTANCE;
        INSTANCE = nullptr;
    }
}

void Android::init() {
    android::Property::Init();
    sdk = android::Property::GetInt32("ro.build.version.sdk");
    id = android::Property::Get("ro.build.id", INVALID_VALUE);
    name = android::Property::Get("ro.product.name", INVALID_VALUE);
    model = android::Property::Get("ro.product.model", INVALID_VALUE);
    manufacturer = android::Property::Get("ro.product.manufacturer", INVALID_VALUE);
    brand = android::Property::Get("ro.product.brand", INVALID_VALUE);
    hardware = android::Property::Get("ro.hardware", INVALID_VALUE);
    abis = android::Property::Get("ro.product.cpu.abilist", INVALID_VALUE);
    incremental = android::Property::Get("ro.build.version.incremental", INVALID_VALUE);
    release = android::Property::Get("ro.build.version.release", INVALID_VALUE);
    security = android::Property::Get("ro.build.version.security_patch", INVALID_VALUE);
    type = android::Property::Get("ro.build.type", INVALID_VALUE);
    tag = android::Property::Get("ro.build.tags", INVALID_VALUE);
    fingerprint = android::Property::Get("ro.build.fingerprint", INVALID_VALUE);
    time = android::Property::Get("ro.build.date.utc", INVALID_VALUE);
    debuggable = android::Property::Get("ro.debuggable", INVALID_VALUE);
}

void Android::Dump() {
    LOGI("Android env:\n");
    LOGI("  * ID: %s\n", Id());
    LOGI("  * Name: %s\n", Name());
    LOGI("  * Model: %s\n", Model());
    LOGI("  * Manufacturer: %s\n", Manufacturer());
    LOGI("  * Brand: %s\n", Brand());
    LOGI("  * Hardware: %s\n", Hardware());
    LOGI("  * ABIS: %s\n", Abis());
    LOGI("  * Incremental: %s\n", Incremental());
    LOGI("  * Release: %s\n", Release());
    LOGI("  * Security: %s\n", Security());
    LOGI("  * Type: %s\n", Type());
    LOGI("  * Tag: %s\n", Tag());
    LOGI("  * Fingerprint: %s\n", Fingerprint());
    LOGI("  * Time: %s\n", Time());
    LOGI("  * Debuggable: %s\n", Debuggable());
    LOGI("  * Sdk: %d\n", Sdk());
}
