/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "command/cmd_core.h"
#include "properties/property.h"
#include "api/core.h"

int CoreCommand::main(int argc, char* const argv[]) {
    bool ret = CoreApi::Load(argv[0]);
    if (ret) {
        // symbols init for later
        android::Property::Init();

        LOGI("  * ID: %s\n", android::Property::Get("ro.build.id", "<unknown>").c_str());
        LOGI("  * Name: %s\n", android::Property::Get("ro.product.name", "<unknown>").c_str());
        LOGI("  * Model: %s\n", android::Property::Get("ro.product.model", "<unknown>").c_str());
        LOGI("  * Manufacturer: %s\n", android::Property::Get("ro.product.manufacturer", "<unknown>").c_str());
        LOGI("  * Brand: %s\n", android::Property::Get("ro.product.brand", "<unknown>").c_str());
        LOGI("  * Hardware: %s\n", android::Property::Get("ro.hardware", "<unknown>").c_str());
        LOGI("  * ABIS: %s\n", android::Property::Get("ro.product.cpu.abilist", "<unknown>").c_str());
        LOGI("  * Incremental: %s\n", android::Property::Get("ro.build.version.incremental", "<unknown>").c_str());
        LOGI("  * Release: %s\n", android::Property::Get("ro.build.version.release", "<unknown>").c_str());
        LOGI("  * Security: %s\n", android::Property::Get("ro.build.version.security_patch", "<unknown>").c_str());
        LOGI("  * Type: %s\n", android::Property::Get("ro.build.type", "<unknown>").c_str());
        LOGI("  * Tag: %s\n", android::Property::Get("ro.build.tags", "<unknown>").c_str());
        LOGI("  * Fingerprint: %s\n", android::Property::Get("ro.build.fingerprint", "<unknown>").c_str());
        LOGI("  * Time: %s\n", android::Property::Get("ro.build.date.utc", "<unknown>").c_str());
        LOGI("  * Debuggable: %s\n", android::Property::Get("ro.debuggable", "<unknown>").c_str());
        LOGI("  * Sdk: %s\n", android::Property::Get("ro.build.version.sdk", "<unknown>").c_str());
    }
    return ret;
}

void CoreCommand::usage() {
    LOGI("Usage: core /tmp/default.core\n");
}
