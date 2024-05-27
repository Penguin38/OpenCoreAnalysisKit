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

#ifndef ANDROID_ANDROID_H_
#define ANDROID_ANDROID_H_

#include "api/core.h"
#include "runtime/runtime.h"
#include "runtime/art_field.h"
#include "runtime/art_method.h"
#include "runtime/mirror/class.h"
#include <stdint.h>
#include <sys/types.h>
#include <functional>
#include <string>
#include <vector>
#include <map>
#include <memory>

#define INVALID_VALUE "<unknown>"

class Android {
public:
    static constexpr int N = 24;
    static constexpr int N_MR1 = 25;
    static constexpr int O = 26;
    static constexpr int O_MR1 = 27;
    static constexpr int P = 28;
    static constexpr int Q = 29;
    static constexpr int R = 30; // base target
    static constexpr int S = 31;
    static constexpr int S_V2 = 32;
    static constexpr int TIRAMISU = 33;
    static constexpr int UPSIDE_DOWN_CAKE = 34;
    static constexpr int VANILLA_ICE_CREAM = 35;

    enum BasicType {
        basic_object = 2,
        basic_boolean = 4,
        basic_char = 5,
        basic_float = 6,
        basic_double = 7,
        basic_byte = 8,
        basic_short = 9,
        basic_int = 10,
        basic_long = 11,
    };

    static Android::BasicType SignatureToBasicTypeAndSize(const char* sig, uint64_t* size_out);
    static Android::BasicType SignatureToBasicTypeAndSize(const char* sig, uint64_t* size_out, const char* def);

    inline static const char *LIBART64 = "/apex/com.android.art/lib64/libart.so";
    inline static const char *LIBART32 = "/apex/com.android.art/lib/libart.so";
    inline static const char *LIBART64_LV29 = "/apex/com.android.runtime/lib64/libart.so";
    inline static const char *LIBART32_LV29 = "/apex/com.android.runtime/lib/libart.so";
    inline static const char* ART_RUNTIME_INSTANCE = "_ZN3art7Runtime9instance_E";

    static Android* INSTANCE;
    static bool IsReady() { return INSTANCE != nullptr; }
    static bool IsSdkReady() { return IsReady() && Sdk() >= Q; }
    static void Init();
    static void Clean();
    static void Dump();
    static int Sdk() { return INSTANCE->sdk; }
    static const char* Id() { return INSTANCE->id.c_str(); }
    static const char* Name() { return INSTANCE->name.c_str(); }
    static const char* Model() { return INSTANCE->model.c_str(); }
    static const char* Manufacturer() { return INSTANCE->manufacturer.c_str(); }
    static const char* Brand() { return INSTANCE->brand.c_str(); }
    static const char* Hardware() { return INSTANCE->hardware.c_str(); }
    static const char* Abis() { return INSTANCE->abis.c_str(); }
    static const char* Incremental() { return INSTANCE->incremental.c_str(); }
    static const char* Release() { return INSTANCE->release.c_str(); }
    static const char* Security() { return INSTANCE->security.c_str(); }
    static const char* Type() { return INSTANCE->type.c_str(); }
    static const char* Tag() { return INSTANCE->tag.c_str(); }
    static const char* Fingerprint() { return INSTANCE->fingerprint.c_str(); }
    static const char* Time() { return INSTANCE->time.c_str(); }
    static const char* Debuggable() { return INSTANCE->debuggable.c_str(); }

    // Configure
    static void OnSdkChanged(int sdk);

    class SdkListener {
    public:
        SdkListener(int sdk, std::function<void ()> fn) : minisdk(sdk) { init = fn; }
        int minisdk;
        std::function<void ()> init;
        void execute(int sdk) {
            if (sdk >= minisdk) {
                init();
            }
        }
    };
    static void RegisterSdkListener(int minisdk, std::function<void ()> fn);

    // API
    static void Prepare();
    static void ForeachInstanceField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn);
    static void ForeachStaticField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn);
    static void ForeachArtMethods(art::mirror::Class& clazz, std::function<bool (art::ArtMethod& method)> fn);
    inline static art::Runtime& GetRuntime() { return INSTANCE->current(); }
    static void SysRoot(const char* path);
    inline static uint64_t SearchSymbol(const char* symbol) { return CoreApi::SearchSymbol(INSTANCE->realLibart.c_str(), symbol); }
    static void ForeachObjects(std::function<bool (art::mirror::Object& object)> fn);

    static constexpr int EACH_APP_OBJECTS = 1 << 0;
    static constexpr int EACH_ZYGOTE_OBJECTS = 1 << 1;
    static constexpr int EACH_IMAGE_OBJECTS = 1 << 2;
    static constexpr int EACH_FAKE_OBJECTS = 1 << 3;
    /*
     * app
     * zygote
     * image
     */
    static void ForeachObjects(std::function<bool (art::mirror::Object& object)> fn, int flag);
private:
    void init();
    void onSdkChanged(int sdk);
    void preLoad();
    void preLoadLater();
    inline art::Runtime& current() { return instance_; }

    int sdk;
    std::string id;
    std::string name;
    std::string model;
    std::string manufacturer;
    std::string brand;
    std::string hardware;
    std::string abis;
    std::string incremental;
    std::string release;
    std::string security;
    std::string type;
    std::string tag;
    std::string fingerprint;
    std::string time;
    std::string debuggable;

    art::Runtime instance_;
    std::string realLibart;
protected:
    std::vector<std::unique_ptr<SdkListener>> mSdkListeners;
};

#endif // ANDROID_ANDROID_H_
