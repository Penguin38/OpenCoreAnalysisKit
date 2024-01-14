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

#include "command/cmd_core.h"
#include "properties/property.h"
#include "api/core.h"

int CoreCommand::main(int argc, char* const argv[]) {
    bool ret = CoreApi::Load(argv[0]);
    if (ret) {
        // symbols init for later
        android::Property::Init();

        std::cout << "  * ID: " << android::Property::Get("ro.build.id", "<unknown>") << std::endl;
        std::cout << "  * Name: " << android::Property::Get("ro.product.name", "<unknown>") << std::endl;
        std::cout << "  * Model: " << android::Property::Get("ro.product.model", "<unknown>") << std::endl;
        std::cout << "  * Manufacturer: " << android::Property::Get("ro.product.manufacturer", "<unknown>") << std::endl;
        std::cout << "  * Brand: " << android::Property::Get("ro.product.brand", "<unknown>") << std::endl;
        std::cout << "  * Hardware: " << android::Property::Get("ro.hardware", "<unknown>") << std::endl;
        std::cout << "  * ABIS: " << android::Property::Get("ro.product.cpu.abilist", "<unknown>") << std::endl;
        std::cout << "  * Incremental: " << android::Property::Get("ro.build.version.incremental", "<unknown>") << std::endl;
        std::cout << "  * Release: " << android::Property::Get("ro.build.version.release", "<unknown>") << std::endl;
        std::cout << "  * Security: " << android::Property::Get("ro.build.version.security_patch", "<unknown>") << std::endl;
        std::cout << "  * Type: " << android::Property::Get("ro.build.type", "<unknown>") << std::endl;
        std::cout << "  * Tag: " << android::Property::Get("ro.build.tags", "<unknown>") << std::endl;
        std::cout << "  * Fingerprint: " << android::Property::Get("ro.build.fingerprint", "<unknown>") << std::endl;
        std::cout << "  * Time: " << android::Property::Get("ro.build.date.utc", "<unknown>") << std::endl;
        std::cout << "  * Debuggable: " << android::Property::Get("ro.debuggable", "<unknown>") << std::endl;
        std::cout << "  * Sdk: " << android::Property::Get("ro.build.version.sdk", "<unknown>") << std::endl;
    }
    return ret;
}

void CoreCommand::usage() {
    std::cout << "Usage: core /tmp/default.core" << std::endl;
}
