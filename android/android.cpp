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
#include "zip/zip_file.h"
#include "base/utils.h"
#include "common/bit.h"
#include "android.h"
#include "properties/property.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/string.h"
#include "runtime/mirror/array.h"
#include "runtime/mirror/dex_cache.h"
#include "runtime/runtime.h"
#include "runtime/image.h"
#include "runtime/class_linker.h"
#include "runtime/indirect_reference_table.h"
#include "runtime/vdex_file.h"
#include "runtime/managed_stack.h"
#include "runtime/oat_quick_method_header.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/space.h"
#include "runtime/gc/space/region_space.h"
#include "runtime/gc/space/image_space.h"
#include "runtime/gc/space/zygote_space.h"
#include "runtime/gc/space/large_object_space.h"
#include "runtime/gc/accounting/space_bitmap.h"
#include "runtime/jni/java_vm_ext.h"
#include "runtime/oat/oat_file.h"
#include "runtime/oat/stack_map.h"
#include "runtime/interpreter/shadow_frame.h"
#include "runtime/jit/jit.h"
#include "runtime/jit/jit_code_cache.h"
#include "runtime/jit/jit_memory_region.h"
#include "runtime/entrypoints/runtime_asm_entrypoints.h"
#include "dex/dex_file.h"
#include "dex/dex_file_structs.h"
#include "dex/standard_dex_file.h"
#include "dex/compact_dex_file.h"
#include "base/length_prefixed_array.h"
#include "base/mem_map.h"

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
    switch (c) {
        case '[':
        case 'L':
        case 'Z':
        case 'C':
        case 'F':
        case 'D':
        case 'B':
        case 'S':
        case 'I':
        case 'J':
        case 'V':
            return SignatureToBasicTypeAndSize(sig, size_out);
    }
    return SignatureToBasicTypeAndSize(def, size_out);
}

void Android::Init() {
    if (!INSTANCE) {
        INSTANCE = new Android();
        INSTANCE->init();
    }
}

void Android::Clean() {
    if (INSTANCE) {
        ResetOatVersion();
        if (INSTANCE->instance_.Ptr()) {
            INSTANCE->instance_.CleanCache();
            INSTANCE->instance_ = 0x0;
            INSTANCE->mSdkListeners.clear();
            INSTANCE->mOatListeners.clear();
        }
        delete INSTANCE;
        INSTANCE = nullptr;
    }
}

void Android::init() {
    preLoad();
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
    CoreApi::RegisterSysRootListener(OnLibartLoad);
}

void Android::preLoad() {
    android::Property::Init();

    art::ArtField::Init();
    art::LengthPrefixedArray::Init();
    art::ThreadList::Init();
    art::OatFile::Init();
    art::OatDexFile::Init();
    art::VdexFile::Init();
    art::MemMap::Init();
    art::ManagedStack::Init();
    art::ShadowFrame::Init();
    art::StandardDexFile::CodeItem::Init();
    art::CompactDexFile::CodeItem::Init();

    art::mirror::Object::Init();
    art::mirror::Class::Init();
    art::mirror::String::Init();
    art::mirror::Array::Init();

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
    art::gc::space::LargeObjectMapSpace::LargeObject::Init();
    art::gc::space::LargeObjectMapSpace::LargeObjectsPair::Init();
    art::gc::space::AllocationInfo::Init();
    art::gc::space::FreeListSpace::Init();

    // preLoadLater listener
    // 26
    RegisterSdkListener(O, art::DexFile::Init26);
    RegisterSdkListener(O, art::Runtime::Init26);
    RegisterSdkListener(O, art::Thread::Init26);
    RegisterSdkListener(O, art::Thread::tls_ptr_sized_values::Init26);
    RegisterSdkListener(O, art::Thread::tls_32bit_sized_values::Init26);
    RegisterSdkListener(O, art::ImageHeader::Init26);
    RegisterSdkListener(O, art::mirror::DexCache::Init26);
    RegisterSdkListener(O, art::gc::space::RegionSpace::Init26);
    RegisterSdkListener(O, art::gc::space::RegionSpace::Region::Init26);
    RegisterSdkListener(O, art::gc::space::LargeObjectSpace::Init26);
    RegisterSdkListener(O, art::gc::space::LargeObjectMapSpace::Init26);
    RegisterSdkListener(O, art::JavaVMExt::Init28);
    RegisterSdkListener(O, art::IndirectReferenceTable::Init26);
    RegisterSdkListener(O, art::ClassLinker::Init26);
    RegisterSdkListener(O, art::ClassLinker::DexCacheData::Init26);
    RegisterSdkListener(O, art::IrtEntry::Init26);
    RegisterSdkListener(O, art::ArtMethod::Init26);
    RegisterSdkListener(O, art::ArtMethod::PtrSizedFields::Init26);
    RegisterSdkListener(O, art::gc::accounting::ContinuousSpaceBitmap::Init26);
    RegisterSdkListener(O, art::OatQuickMethodHeader::Init26);
    RegisterSdkListener(O, art::jit::Jit::Init26);
    RegisterSdkListener(O, art::jit::JitCodeCache::Init26);

    // 28
    RegisterSdkListener(P, art::DexFile::Init28);
    RegisterSdkListener(P, art::Runtime::Init28);
    RegisterSdkListener(P, art::Thread::Init28);
    RegisterSdkListener(P, art::ImageHeader::Init28);
    RegisterSdkListener(P, art::gc::space::RegionSpace::Init28);
    RegisterSdkListener(P, art::gc::space::LargeObjectMapSpace::Init28);
    RegisterSdkListener(P, art::JavaVMExt::Init28);
    RegisterSdkListener(P, art::ClassLinker::Init28);
    RegisterSdkListener(P, art::ClassLinker::DexCacheData::Init28);
    RegisterSdkListener(P, art::ArtMethod::Init28);
    RegisterSdkListener(P, art::ArtMethod::PtrSizedFields::Init28);
    RegisterSdkListener(P, art::jit::JitCodeCache::Init28);
    RegisterSdkListener(P, art::jit::JitCodeCache::JniStubData::Init28);
    RegisterSdkListener(P, art::jit::JitCodeCache::JniStubsMapPair::Init28);

    // 29
    RegisterSdkListener(Q, art::DexFile::Init29);
    RegisterSdkListener(Q, art::Runtime::Init29);
    RegisterSdkListener(Q, art::Thread::Init29);
    RegisterSdkListener(Q, art::ImageHeader::Init29);
    RegisterSdkListener(Q, art::gc::space::RegionSpace::Init29);
    RegisterSdkListener(Q, art::gc::space::RegionSpace::Region::Init29);
    RegisterSdkListener(Q, art::gc::space::LargeObjectSpace::Init29);
    RegisterSdkListener(Q, art::gc::space::LargeObjectMapSpace::Init29);
    RegisterSdkListener(Q, art::JavaVMExt::Init29);
    RegisterSdkListener(Q, art::IndirectReferenceTable::Init29);
    RegisterSdkListener(Q, art::gc::accounting::ContinuousSpaceBitmap::Init29);
    RegisterSdkListener(Q, art::OatQuickMethodHeader::Init29);
    RegisterSdkListener(Q, art::jit::Jit::Init29);
    RegisterSdkListener(Q, art::jit::JitCodeCache::Init29);

    // 30 base
    RegisterSdkListener(R, art::Runtime::Init30);
    RegisterSdkListener(R, art::mirror::DexCache::Init30);
    RegisterSdkListener(R, art::Thread::Init30);
    RegisterSdkListener(R, art::Thread::tls_ptr_sized_values::Init30);
    RegisterSdkListener(R, art::gc::space::RegionSpace::Init30);
    RegisterSdkListener(R, art::gc::space::LargeObjectSpace::Init30);
    RegisterSdkListener(R, art::gc::space::LargeObjectMapSpace::Init30);
    RegisterSdkListener(R, art::jit::JitCodeCache::Init30);
    RegisterSdkListener(R, art::jit::JitMemoryRegion::Init30);
    RegisterSdkListener(R, art::jit::ZygoteMap::Init30);

    // 31
    RegisterSdkListener(S, art::Runtime::Init31);
    RegisterSdkListener(S, art::ImageHeader::Init31);
    RegisterSdkListener(S, art::Thread::Init31);
    RegisterSdkListener(S, art::Thread::tls_32bit_sized_values::Init31);
    RegisterSdkListener(S, art::gc::space::RegionSpace::Init31);
    RegisterSdkListener(S, art::JavaVMExt::Init31);
    RegisterSdkListener(S, art::ArtMethod::Init31);
    RegisterSdkListener(S, art::OatQuickMethodHeader::Init31);
    RegisterSdkListener(S, art::jit::JitCodeCache::Init31);
    RegisterSdkListener(S, art::OatDexFile::Init31);

    // 33
    RegisterSdkListener(TIRAMISU, art::Runtime::Init33);
    RegisterSdkListener(TIRAMISU, art::Thread::Init33);
    RegisterSdkListener(TIRAMISU, art::JavaVMExt::Init33);
    RegisterSdkListener(TIRAMISU, art::ClassLinker::DexCacheData::Init33);
    RegisterSdkListener(TIRAMISU, art::IrtEntry::Init33);

    // 34
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::Runtime::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::DexFile::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::ImageHeader::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::Thread::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::Thread::tls_ptr_sized_values::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::JavaVMExt::Init34);
    RegisterSdkListener(UPSIDE_DOWN_CAKE, art::IndirectReferenceTable::Init34);

    // OAT
    RegisterOatListener(124, art::OatQuickMethodHeader::OatInit124);
    RegisterOatListener(124, art::CodeInfo::OatInit124);
    RegisterOatListener(150, art::CodeInfo::OatInit150);
    RegisterOatListener(156, art::OatQuickMethodHeader::OatInit156);
    RegisterOatListener(158, art::OatQuickMethodHeader::OatInit158);
    RegisterOatListener(171, art::CodeInfo::OatInit171);
    RegisterOatListener(172, art::CodeInfo::OatInit172);
    RegisterOatListener(191, art::CodeInfo::OatInit191);
    RegisterOatListener(192, art::OatQuickMethodHeader::OatInit192);
    RegisterOatListener(238, art::OatQuickMethodHeader::OatInit238);
    RegisterOatListener(239, art::OatQuickMethodHeader::OatInit239);
}

void Android::preLoadLater() {
    if (Sdk() > Q) {
        realLibart = (CoreApi::Bits() == 64) ? LIBART64 : LIBART32;
    } else {
        if (Sdk() > P) {
            realLibart = (CoreApi::Bits() == 64) ? LIBART64_LV29 : LIBART32_LV29;
        } else {
            realLibart = (CoreApi::Bits() == 64) ? LIBART64_LV28 : LIBART32_LV28;
        }
    }

    LOGI("Switch android(%d) env.\n", sdk);
    for (const auto& listener : mSdkListeners) {
        listener->execute(sdk);
    }
}

void Android::oatPreLoadLater() {
    LOGI("Switch oat version(%d) env.\n", oat_header_.kOatVersion);
    for (const auto& listener : mOatListeners) {
        listener->execute(oat_header_.kOatVersion);
    }
}

void Android::RegisterSdkListener(int minisdk, std::function<void ()> fn) {
    std::unique_ptr<Android::SdkListener> listener = std::make_unique<Android::SdkListener>(minisdk, fn);
    INSTANCE->mSdkListeners.push_back(std::move(listener));
}

void Android::RegisterOatListener(int minioat, std::function<void ()> fn) {
    std::unique_ptr<Android::OatListener> listener = std::make_unique<Android::OatListener>(minioat, fn);
    INSTANCE->mOatListeners.push_back(std::move(listener));
}

void Android::OnSdkChanged(int sdk) {
    if (sdk < O) {
        LOGE("ERROR: Invaild sdk(%d)\n", sdk);
        return;
    }
    INSTANCE->onSdkChanged(sdk);
}

void Android::OnOatChanged(int oat) {
    if (oat > 999) {
        LOGE("ERROR: Invaild oat(%d)\n", oat);
        return;
    }
    INSTANCE->onOatChanged(oat);
}

void Android::onSdkChanged(int current_sdk) {
    if (sdk != current_sdk) {
        sdk = current_sdk;
        preLoadLater();
        if (instance_.Ptr()) {
            instance_.CleanCache();
            instance_ = 0x0;
        }
    }
}

void Android::onOatChanged(int current_oat) {
    if (oat != current_oat) {
        oat = current_oat;
        oatPreLoadLater();
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

void Android::ForeachArtMethods(art::mirror::Class& clazz, std::function<bool (art::ArtMethod& method)> fn) {
    uint32_t size = clazz.NumMethods();
    if (!size) return;
    art::ArtMethod method(clazz.GetMethods(), clazz);
    int i = 0;
    do {
        if (fn(method)) break;
        i++;
        if (i < size) {
            method.MovePtr(SIZEOF(ArtMethod));
        } else {
            break;
        }
    } while(true);
}

void Android::ForeachVirtualArtMethods(art::mirror::Class& clazz, std::function<bool (art::ArtMethod& method)> fn) {
    uint32_t size = clazz.NumMethods();
    uint32_t virtual_offset = clazz.NumDirectMethods();
    if (!(size - virtual_offset)) return;
    art::ArtMethod method(clazz.GetMethods(), clazz);
    int i = virtual_offset;
    do {
        if (fn(method)) break;
        i++;
        if (i < size) {
            method.MovePtr(SIZEOF(ArtMethod));
        } else {
            break;
        }
    } while(true);
}

void Android::ForeachObjects(std::function<bool (art::mirror::Object& object)> fn) {
    ForeachObjects(fn, EACH_IMAGE_OBJECTS | EACH_ZYGOTE_OBJECTS | EACH_APP_OBJECTS | EACH_FAKE_OBJECTS);
}

void Android::ForeachObjects(std::function<bool (art::mirror::Object& object)> fn, int flag) {
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
        if (space->IsImageSpace()) {
            if (flag & EACH_IMAGE_OBJECTS) walkfn(space.get());
        } else if (space->IsZygoteSpace()) {
            if (flag & EACH_ZYGOTE_OBJECTS) walkfn(space.get());
        } else if (space->IsRegionSpace()) {
            if (flag & EACH_APP_OBJECTS) walkfn(space.get());
        } else if (space->IsFakeSpace()) {
            if (flag & EACH_FAKE_OBJECTS) walkfn(space.get());
        } else {
            if (space->GetType() != art::gc::space::kSpaceTypeInvalidSpace) {
                walkfn(space.get());
            } else {
                LOGE("ERROR: please run sysroot libart.so, %s invalid space.\n", space->GetName());
            }
        }
    }

    for (const auto& space : heap.GetDiscontinuousSpaces()) {
        if (flag & EACH_APP_OBJECTS) walkfn(space.get());
    }
}

void Android::SysRoot(const char* path) {
    std::vector<char *> dirs;
    std::unique_ptr<char> newpath(strdup(path));
    char *token = strtok(newpath.get(), ":");
    while (token != nullptr) {
        dirs.push_back(token);
        token = strtok(nullptr, ":");
    }

    art::Runtime& runtime = art::Runtime::Current();
    art::ClassLinker& linker = runtime.GetClassLinker();

    for (const auto& value : linker.GetDexCacheDatas()) {
        art::mirror::DexCache& dex_cache = value->GetDexCache();
        art::DexFile& dex_file = value->GetDexFile();
        std::string name;
        if (dex_cache.Ptr()) {
            name = dex_cache.GetLocation().ToModifiedUtf8();
        } else if (dex_file.Ptr()) {
            name = dex_file.GetLocation().c_str();
        }
        LoadBlock* block = CoreApi::FindLoadBlock(dex_file.data_begin(), false);
        if (block) {
            bool isVdex = false;
            art::OatDexFile& oat_dex_file = dex_file.GetOatDexFile();
            if (oat_dex_file.Ptr()) {
                art::OatFile& oat_file = oat_dex_file.GetOatFile();
                if (oat_file.Ptr() && block->virtualContains(oat_file.GetVdexBegin())) {
                    name = oat_file.GetVdexFile().GetName();
                    isVdex = true;
                }
            }

            std::unique_ptr<char> newname(strdup(name.c_str()));
            char *ori_dex_file = strtok(newname.get(), "!");
            char *sub_file = strtok(NULL, "!");

            std::string filepath;
            for (char *dir : dirs) {
                if (Utils::SearchFile(dir, &filepath, ori_dex_file))
                    break;
            }

            if (!filepath.length())
                continue;

            if (isVdex) {
                std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(filepath.c_str()));
                if (map) {
                    if (memcmp((void*)map->data(), "vdex", 4)) {
                        LOGE("ERROR: Invalid vdex file (%s)\n", filepath.c_str());
                        continue;
                    }
                    block->setMmapFile(filepath.c_str(), 0x0);
                }
            } else {
                ZipFile zip;
                if (zip.open(filepath.c_str())) {
                    LOGE("ERROR: Zip open fail [%lx] %s\n", block->vaddr(), ori_dex_file);
                    continue;
                }

                ZipEntry* entry;
                if (!sub_file) {
                    entry = zip.getEntryByName("classes.dex");
                } else {
                    entry = zip.getEntryByName(sub_file);
                }

                if (!entry) {
                    LOGE("ERROR: %s Not found entry %s\n", filepath.c_str(), sub_file ? sub_file : "classes.dex");
                    continue;
                }

                if (!entry->IsUncompressed()) {
                    LOGE("ERROR: Not support compress zip [%lx] %s!%s\n", block->vaddr(), filepath.c_str(), sub_file ? sub_file : "classes.dex");
                    continue;
                }

                block->setMmapFile(filepath.c_str(), RoundDown(entry->getFileOffset(), 0x1000));
            }
        } else {
            LOGE("ERROR: Unknown DexCache(0x%lx) %s region\n", dex_cache.Ptr(), dex_cache.GetLocation().ToModifiedUtf8().c_str());
        }
    }
}

void Android::Prepare() {
    try {
        if (!CoreApi::IsReady() || !IsSdkReady())
            return;

        if (!INSTANCE->instance_.Ptr()) {
            art::Runtime& runtime = art::Runtime::Current();
            art::gc::Heap& heap = runtime.GetHeap();
            heap.GetContinuousSpaces();
            heap.GetDiscontinuousSpaces();
        }
    } catch (InvalidAddressException e) {}
}

void Android::OatPrepare() {
    OnOatChanged(art::OatHeader::OatVersion());
}

void Android::onLibartLoad(LinkMap *map) {
    if (realLibart == map->name()) {
        art::EntryPoints::Init();
    }
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
