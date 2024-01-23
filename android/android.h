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

#include "runtime/art_field.h"
#include "runtime/mirror/class.h"
#include <stdint.h>
#include <sys/types.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>

#define INVALID_VALUE "<unknown>"

class Android {
public:
    static constexpr int R = 30;
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

    static Android* INSTANCE;
    static bool IsReady() { return INSTANCE != nullptr; }
    static bool IsSdkReady() { return IsReady() && Sdk() >= R; }
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
    static void ForeachInstanceField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn);
    static void ForeachStaticField(art::mirror::Class& clazz, std::function<bool (art::ArtField& field)> fn);
private:
    void init();
    void onSdkChanged(int sdk);
    void preLoad();
    void preLoadLater();

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
protected:
    std::vector<std::unique_ptr<SdkListener>> mSdkListeners;
};

#endif // ANDROID_ANDROID_H_
