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
#include "runtime/mirror/string.h"
#include "runtime/mirror/array.h"
#include "runtime/mirror/dex_cache.h"
#include "runtime/runtime.h"
#include "runtime/image.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/space.h"
#include "runtime/gc/space/region_space.h"
#include "runtime/gc/space/image_space.h"
#include "runtime/gc/space/zygote_space.h"
#include "runtime/gc/space/large_object_space.h"
#include "runtime/gc/accounting/space_bitmap.h"
#include "dex/dex_file.h"
#include "dex/dex_file_structs.h"
#include "base/length_prefixed_array.h"

Android* Android::INSTANCE = nullptr;

Android::BasicType Android::SignatureToBasicTypeAndSize(const char* sig, uint64_t* size_out) {
    char c = sig[0];
    Android::BasicType ret;
    uint64_t size;

    switch (c) {
        case '[':
        case 'L':
            ret = basic_object;
            size = 4;
            break;
        case 'Z':
            ret = basic_boolean;
            size = 1;
            break;
        case 'C':
            ret = basic_char;
            size = 2;
            break;
        case 'F':
            ret = basic_float;
            size = 4;
            break;
        case 'D':
            ret = basic_double;
            size = 8;
            break;
        case 'B':
            ret = basic_byte;
            size = 1;
            break;
        case 'S':
            ret = basic_short;
            size = 2;
            break;
        case 'I':
            ret = basic_int;
            size = 4;
            break;
        case 'J':
            ret = basic_long;
            size = 8;
            break;
        default:
            __builtin_unreachable();
    }

    if (size_out != nullptr) {
        *size_out = size;
    }
    return ret;
}

Android::BasicType Android::SignatureToBasicTypeAndSize(const char* sig, uint64_t* size_out, const char* def) {
    char c = sig[0];
    if (c == '\0') {
        return SignatureToBasicTypeAndSize(def, size_out);
    } else {
        return SignatureToBasicTypeAndSize(sig, size_out);
    }
}

void Android::Init() {
    if (!INSTANCE) {
        INSTANCE = new Android();
        INSTANCE->init();
    }
}

void Android::Clean() {
    if (INSTANCE) {
        if (INSTANCE->instance_.Ptr())
            INSTANCE->instance_.CleanCache();
        delete INSTANCE;
        INSTANCE = nullptr;
    }
}

void Android::init() {
    preLoad();
    realLibart = (CoreApi::GetPointSize() == 64) ? LIBART64 : LIBART32;
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
    preLoadLater();
}

void Android::preLoad() {
    android::Property::Init();

    art::ArtField::Init();
    art::LengthPrefixedArray::Init();

    art::mirror::Object::Init();
    art::mirror::Class::Init();
    art::mirror::String::Init();
    art::mirror::Array::Init();
    art::mirror::DexCache::Init();

    art::dex::TypeId::Init();
    art::dex::StringId::Init();
    art::dex::FieldId::Init();
    art::dex::MethodId::Init();
    art::dex::ProtoId::Init();
    art::dex::TypeList::Init();
    art::dex::TypeItem::Init();

    art::gc::Heap::Init();
    art::gc::space::Space::Init();
    art::gc::space::ContinuousSpace::Init();
    art::gc::space::RegionSpace::Region::Init();
    art::gc::space::LargeObjectSpace::Init();
    art::gc::space::LargeObjectMapSpace::Init();
    art::gc::space::LargeObjectMapSpace::LargeObject::Init();
    art::gc::space::LargeObjectMapSpace::LargeObjectsPair::Init();
    art::gc::space::AllocationInfo::Init();
    art::gc::space::FreeListSpace::Init();
    art::gc::accounting::ContinuousSpaceBitmap::Init();

    // preLoadLater listener
    RegisterSdkListener(S, art::Runtime::Init31);
    RegisterSdkListener(S, art::ImageHeader::Init31);
    RegisterSdkListener(S, art::gc::space::RegionSpace::Init31);

    RegisterSdkListener(TIRAMISU, art::Runtime::Init33);

    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::DexFile::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::ImageHeader::Init34);
}

void Android::preLoadLater() {
    art::DexFile::Init();
    art::Runtime::Init();
    art::ImageHeader::Init();
    art::gc::space::RegionSpace::Init();

    LOGI("Switch android(%d) env.\n", sdk);
    for (const auto& listener : mSdkListeners) {
        listener->execute(sdk);
    }
}

void Android::RegisterSdkListener(int minisdk, std::function<void ()> fn) {
    std::unique_ptr<Android::SdkListener> listener = std::make_unique<Android::SdkListener>(minisdk, fn);
    INSTANCE->mSdkListeners.push_back(std::move(listener));
}

void Android::OnSdkChanged(int sdk) {
    if (sdk < R) {
        LOGI("Invaild sdk(%d)\n", sdk);
        return;
    }
    INSTANCE->onSdkChanged(sdk);
}

void Android::onSdkChanged(int current_sdk) {
    if (sdk != current_sdk) {
        sdk = current_sdk;
        preLoadLater();
    }
}

void Android::ForeachInstanceField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn) {
    uint32_t size = clazz.NumInstanceFields();
    if (!size) return;
    art::ArtField field(clazz.GetIFields(), clazz);
    int i = 0;
    do {
        if (fn(field)) break;
        i++;
        if (i < size) {
            field.MovePtr(SIZEOF(ArtField));
        } else {
            break;
        }
    } while(true);
}

void Android::ForeachStaticField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn) {
    uint32_t size = clazz.NumStaticFields();
    if (!size) return;
    art::ArtField field(clazz.GetSFields(), clazz);
    int i = 0;
    do {
        if (fn(field)) break;
        i++;
        if (i < size) {
            field.MovePtr(SIZEOF(ArtField));
        } else {
            break;
        }
    } while(true);
}

void Android::ForeachObjects(std::function<bool (art::mirror::Object& object)> fn) {
    art::Runtime& runtime = art::Runtime::Current();
    art::gc::Heap& heap = runtime.GetHeap();

    auto walkfn = [&fn](art::gc::space::Space* space) {
        LOGD("Walk [%s] ...\n", space->GetName());
        if (space->IsVaildSpace()) {
            space->Walk(fn);
        } else {
            LOGE("ERROR: %s invalid space.\n", space->GetName());
        }
    };

    for (const auto& space : heap.GetContinuousSpaces()) {
        walkfn(space.get());
    }

    for (const auto& space : heap.GetDiscontinuousSpaces()) {
        walkfn(space.get());
    }
}

void Android::SysRoot(const char* path) {

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
