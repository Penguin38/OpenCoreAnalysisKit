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
#include "command/cmd_getprop.h"
#include "properties/property.h"
#include "base/utils.h"
#include "api/core.h"
#include <iostream>

void GetPropCommand::printProperties() {
    auto callback = [](android::PropInfo& info) {
        if (info.Ptr()) {
            LOGI("[%s]: [%s]\n", info.name().c_str(), info.value().c_str());
        }
    };
    android::Property::Foreach(callback);
}

int GetPropCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    if (!argc) {
        printProperties();
    } else {
        LOGI("%s\n", android::Property::Get(argv[0]).c_str());
    }
    return 0;
}

void GetPropCommand::usage() {
    LOGI("Usage: getprop [NAME]\n");
}
