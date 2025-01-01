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
#include "base/macros.h"
#include "common/bit.h"
#include "common/elf.h"
#include "android.h"
#include "fdtrack/fdtrack.h"
#include "unwindstack/Unwinder.h"
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
#include "runtime/handle_scope.h"
#include "runtime/monitor.h"
#include "runtime/monitor_pool.h"
#include "runtime/base/mutex.h"
#include "runtime/base/locks.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/space.h"
#include "runtime/gc/space/region_space.h"
#include "runtime/gc/space/image_space.h"
#include "runtime/gc/space/zygote_space.h"
#include "runtime/gc/space/large_object_space.h"
#include "runtime/gc/space/bump_pointer_space.h"
#include "runtime/gc/accounting/space_bitmap.h"
#include "runtime/jni/java_vm_ext.h"
#include "runtime/jni/jni_env_ext.h"
#include "runtime/jni/local_reference_table.h"
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
#include "logcat/log.h"
#include <stdio.h>

std::unique_ptr<Android> Android::INSTANCE = nullptr;

void Android::Init() {
    INSTANCE = std::make_unique<Android>();
    INSTANCE->init();
}

Android::~Android() {
    if (instance_.Ptr())
        instance_.CleanCache();
    mSdkListeners.clear();
    mOatListeners.clear();
}

static int kTrunkData[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 8, 8, 9, 10,
    11,12,12,13,14,15,
};

int Android::Sdk2Trunk(int sdk) {
    if (sdk > sizeof(kTrunkData) / sizeof(kTrunkData[0])) {
        return 0;
    }
    return kTrunkData[sdk];
}

void Android::init() {
    preLoad();
    trunk = android::Property::GetInt32("ro.build.version.trunk");
    sdk = android::Property::GetInt32("ro.build.version.sdk");
    if (trunk > Sdk2Trunk(sdk)) {
        LOGW("current trunk(%d) no match sdk(%d).\n", trunk, sdk);
        int trunk_size = sizeof(kTrunkData) / sizeof(kTrunkData[0]);
        for (int cur = sdk + 1; cur < trunk_size; ++cur) {
            if (trunk == kTrunkData[cur]) {
                sdk = cur;
                break;
            }
        }
    }
    release = android::Property::Get("ro.build.version.release", INVALID_VALUE);
    sscanf(release.c_str(), "%*d.%*d.%d", &patch);
    id = android::Property::Get("ro.build.id", INVALID_VALUE);
    name = android::Property::Get("ro.product.name", INVALID_VALUE);
    model = android::Property::Get("ro.product.model", INVALID_VALUE);
    manufacturer = android::Property::Get("ro.product.manufacturer", INVALID_VALUE);
    brand = android::Property::Get("ro.product.brand", INVALID_VALUE);
    hardware = android::Property::Get("ro.hardware", INVALID_VALUE);
    abis = android::Property::Get("ro.product.cpu.abilist", INVALID_VALUE);
    incremental = android::Property::Get("ro.build.version.incremental", INVALID_VALUE);
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
    android::Logcat::Init();
    android::FdTrack::Init();
    android::UnwindStack::Init();

    art::ArtField::Init();
    art::LengthPrefixedArray::Init();
    art::ThreadList::Init();
    art::OatFile::Init();
    art::VdexFile::Init();
    art::MemMap::Init();
    art::ManagedStack::Init();
    art::ShadowFrame::Init();
    art::StandardDexFile::Init();
    art::CompactDexFile::Init();
    art::MonitorPool::Init();

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

    art::gc::space::Space::Init();
    art::gc::space::ContinuousSpace::Init();

    art::HandleScope::Init();
    art::DexFile::Init();
    art::Runtime::Init();
    art::gc::Heap::Init();
    art::Thread::Init();
    art::ImageHeader::Init();
    art::mirror::DexCache::Init();
    art::gc::space::RegionSpace::Init();
    art::gc::space::LargeObjectSpace::Init();
    art::gc::space::BumpPointerSpace::Init();
    art::JavaVMExt::Init();
    art::JNIEnvExt::Init();
    art::IndirectReferenceTable::Init();
    art::jni::LocalReferenceTable::Init();
    art::ClassLinker::Init();
    art::ArtMethod::Init();
    art::gc::accounting::ContinuousSpaceBitmap::Init();
    art::jit::Jit::Init();
    art::jit::JitCodeCache::Init();
    art::jit::JitMemoryRegion::Init();
    art::OatDexFile::Init();
    art::BaseMutex::Init();
    art::LockLevel::Init();
    art::Monitor::Init();

    art::CodeInfo::Init();
    art::OatQuickMethodHeader::Init();
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
    if (sdk < M) {
        LOGE("Invaild sdk(%d)\n", sdk);
        return;
    }
    INSTANCE->onSdkChanged(sdk);
}

void Android::OnOatChanged(int oat) {
    if (oat > 999) {
        LOGE("Invaild oat(%d)\n", oat);
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

/*
 *  +-------+-------+
 *  | klass |flags1 |
 *  +-------+-------+
 *  | idx1  |offset1|
 *  +-------+-------+
 *  | klass |flags2 |
 *  +-------+-------+
 *  | idx2  |offset2|
 *  +-------+-------+
 *  ...
 *  only declaring_class cache and it's same.
 */
void Android::ForeachInstanceField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn) {
    uint32_t size = clazz.NumInstanceFields();
    if (!size) return;
    art::ArtField field(clazz.GetIFields(), clazz);
    int i = 0;
    do {
        if (fn(field)) break;
        field.MovePtr(SIZEOF(ArtField));
        i++;
    } while(i < size);
}

void Android::ForeachStaticField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn) {
    uint32_t size = clazz.NumStaticFields();
    if (!size) return;
    art::ArtField field(clazz.GetSFields(), clazz);
    int i = 0;
    do {
        if (fn(field)) break;
        field.MovePtr(SIZEOF(ArtField));
        i++;
    } while(i < size);
}

void Android::ForeachArtMethods(art::mirror::Class& clazz, std::function<bool (art::ArtMethod& method)> fn) {
    auto visit_methods_fn = [&](uint64_t methods, uint32_t size) {
        if (size) {
            api::MemoryRef base(methods, clazz);
            base.Prepare(false);
            int i = 0;
            do {
                art::ArtMethod method(base.Ptr() + i * SIZEOF(ArtMethod), base);
                if (fn(method)) break;
                i++;
            } while(i < size);
        }
    };

    if (LIKELY(Sdk() >= N))
        visit_methods_fn(clazz.GetMethods(), clazz.NumMethods());
    else {
        visit_methods_fn(clazz.GetDirectMethods(), clazz.NumDirectMethods());
        visit_methods_fn(clazz.GetVirtualMethods(), clazz.NumVirtualMethods());
    }
}

void Android::ForeachVirtualArtMethods(art::mirror::Class& clazz, std::function<bool (art::ArtMethod& method)> fn) {
    auto visit_methods_fn = [&](uint64_t methods, uint32_t size, uint32_t off) {
        if ((size - off) > 0) {
            api::MemoryRef base(methods, clazz);
            base.Prepare(false);
            int i = off;
            do {
                art::ArtMethod method(base.Ptr() + i * SIZEOF(ArtMethod), base);
                if (fn(method)) break;
                i++;
            } while(i < size);
        }
    };

    if (LIKELY(Sdk() >= N))
        visit_methods_fn(clazz.GetMethods(), clazz.NumMethods(), clazz.NumDirectMethods());
    else
        visit_methods_fn(clazz.GetVirtualMethods(), clazz.NumVirtualMethods(), 0x0);
}

void Android::ForeachObjects(std::function<bool (art::mirror::Object& object)> fn) {
    ForeachObjects(fn, EACH_IMAGE_OBJECTS | EACH_ZYGOTE_OBJECTS | EACH_APP_OBJECTS | EACH_FAKE_OBJECTS, false);
}

void Android::ForeachObjects(std::function<bool (art::mirror::Object& object)> fn, int flag, bool check) {
    art::Runtime& runtime = art::Runtime::Current();
    art::gc::Heap& heap = runtime.GetHeap();

    auto walkfn = [&](art::gc::space::Space* space) {
        LOGD("Walk [%s] ...\n", space->GetName());
        if (space->IsVaildSpace()) {
            space->Walk(fn, check);
        } else {
            LOGE("%s invalid space.\n", space->GetName());
        }
    };

    for (const auto& space : heap.GetContinuousSpaces()) {
        if (space->IsImageSpace()) {
            if (flag & EACH_IMAGE_OBJECTS) walkfn(space.get());
        } else if (space->IsZygoteSpace()) {
            if (flag & EACH_ZYGOTE_OBJECTS) walkfn(space.get());
        } else if (space->IsRegionSpace() || space->IsBumpPointerSpace()) {
            if (flag & EACH_APP_OBJECTS) walkfn(space.get());
        } else if (space->IsFakeSpace()) {
            if (flag & EACH_FAKE_OBJECTS) walkfn(space.get());
        } else {
            if (space->GetType() != art::gc::space::kSpaceTypeInvalidSpace) {
                walkfn(space.get());
            } else {
                LOGE("please run sysroot libart.so, %s invalid space.\n", space->GetName());
            }
        }
    }

    for (const auto& space : heap.GetDiscontinuousSpaces()) {
        if (flag & EACH_APP_OBJECTS) walkfn(space.get());
    }
}

void Android::ForeachReferences(std::function<bool (art::mirror::Object& object)> fn) {
    ForeachReferences(fn, EACH_LOCAL_REFERENCES | EACH_GLOBAL_REFERENCES | EACH_WEAK_GLOBAL_REFERENCES);
}

void Android::ForeachReferences(std::function<bool (art::mirror::Object& object)> fn, int flag) {
    auto callback = [&](art::mirror::Object& object, int type, uint64_t idx) -> bool {
        fn(object);
        return false;
    };
    ForeachReferences(callback, flag);
}

void Android::ForeachReferences(std::function<bool (art::mirror::Object& object, int type, uint64_t idx)> fn) {
    ForeachReferences(fn, EACH_LOCAL_REFERENCES | EACH_GLOBAL_REFERENCES | EACH_WEAK_GLOBAL_REFERENCES);
}

void Android::ForeachReferences(std::function<bool (art::mirror::Object& object, int type, uint64_t idx)> fn, int flag) {
    art::Runtime& runtime = art::Runtime::Current();
    art::JavaVMExt& jvm = runtime.GetJavaVM();

    int tid = flag >> EACH_LOCAL_REFERENCES_BY_TID_SHIFT;
    if (flag & EACH_LOCAL_REFERENCES) {
        art::ThreadList& thread_list = runtime.GetThreadList();
        for (const auto& thread : thread_list.GetList()) {
            int current = thread->GetTid();
            if (tid && tid != current)
                continue;

            auto local_fn = [&](art::mirror::Object& object, uint64_t idx) -> bool {
                fn(object, art::IndirectRefKind::kLocal | (current << EACH_LOCAL_REFERENCES_BY_TID_SHIFT), idx);
                return false;
            };

            LOGD("Walk [%d] local references table\n", thread->GetTid());
            art::JNIEnvExt& jni_env = thread->GetJNIEnv();
            if (Sdk() >= U) {
                art::jni::LocalReferenceTable& local = jni_env.GetLocalsTable_v34();
                local.Walk(local_fn);
            } else {
                art::IndirectReferenceTable& local = jni_env.GetLocalsTable();
                local.Walk(local_fn);
            }
        }
    }

    if (flag & EACH_GLOBAL_REFERENCES) {
        auto global_fn = [&](art::mirror::Object& object, uint64_t idx) -> bool {
            fn(object, art::IndirectRefKind::kGlobal, idx);
            return false;
        };

        LOGD("Walk global references table\n");
        art::IndirectReferenceTable& global = jvm.GetGlobalsTable();
        global.Walk(global_fn);
    }

    if (flag & EACH_WEAK_GLOBAL_REFERENCES) {
        auto weak_fn = [&](art::mirror::Object& object, uint64_t idx) -> bool {
            fn(object, art::IndirectRefKind::kWeakGlobal, idx);
            return false;
        };

        LOGD("Walk weak global references table\n");
        art::IndirectReferenceTable& weak_global = jvm.GetWeakGlobalsTable();
        weak_global.Walk(weak_fn);
    }
}

void Android::SysRoot(const char* path) {
    art::Runtime& runtime = art::Runtime::Current();
    if (!runtime.Ptr()) return;

    std::vector<char *> dirs;
    std::unique_ptr<char> newpath(strdup(path));
    char *token = strtok(newpath.get(), ":");
    while (token != nullptr) {
        dirs.push_back(token);
        token = strtok(nullptr, ":");
    }

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
                        LOGE("Invalid vdex file (%s)\n", filepath.c_str());
                        continue;
                    }
                    block->setMmapFile(filepath.c_str(), 0x0);
                }
            } else {
                ZipFile zip;
                if (zip.open(filepath.c_str())) {
                    LOGE("Zip open fail [%" PRIx64 "]" ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, block->vaddr(), filepath.c_str());
                    continue;
                }

                ZipEntry* entry;
                if (!sub_file) {
                    entry = zip.getEntryByName("classes.dex");
                } else {
                    entry = zip.getEntryByName(sub_file);
                }

                if (!entry) {
                    LOGE("%s Not found entry" ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, filepath.c_str(), sub_file ? sub_file : "classes.dex");
                    continue;
                }

                if (!entry->IsUncompressed()) {
                    LOGE("Not support compress zip [%" PRIx64 "] %s!%s\n", block->vaddr(), filepath.c_str(), sub_file ? sub_file : "classes.dex");
                    continue;
                }

                block->setMmapFile(filepath.c_str(), RoundDown(entry->getFileOffset(), CoreApi::GetPageSize()));
            }
        } else {
            LOGE("Unknown DexCache(0x%" PRIx64 ") %s region\n", dex_cache.Ptr(), dex_cache.GetLocation().ToModifiedUtf8().c_str());
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
    if (CoreApi::IsReady() && IsReady()) {
        OnOatChanged(art::OatHeader::OatVersion());
    }
}

void Android::onLibartLoad(LinkMap *map) {
    // do nothing
}

void Android::Dump() {
    if (!IsSdkReady())
        return;

    LOGI(ANSI_COLOR_LIGHTRED "Android env:\n" ANSI_COLOR_RESET);
    LOGI("  * ID: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Id());
    LOGI("  * Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Name());
    LOGI("  * Model: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Model());
    LOGI("  * Manufacturer: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Manufacturer());
    LOGI("  * Brand: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Brand());
    LOGI("  * Hardware: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Hardware());
    LOGI("  * ABIS: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Abis());
    // LOGI("  * Incremental: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Incremental());
    LOGI("  * Release: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Release());
    // LOGI("  * Security: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Security());
    LOGI("  * Type: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Type());
    // LOGI("  * Tag: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Tag());
    LOGI("  * Fingerprint: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Fingerprint());
    LOGI("  * Time: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Time());
    LOGI("  * Debuggable: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, Debuggable());
    LOGI("  * Sdk: " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, Sdk());
}
