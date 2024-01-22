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

#include <stdint.h>
#include <sys/types.h>
#include <string>

#define INVALID_VALUE "<unknown>"

class Android {
public:
    static Android* INSTANCE;
    static bool IsReady() { return INSTANCE != nullptr; }
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
private:
    void init();
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
};

#endif // ANDROID_ANDROID_H_
