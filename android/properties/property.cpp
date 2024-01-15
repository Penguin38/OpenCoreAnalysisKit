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

#include "api/core.h"
#include "properties/prop_area.h"
#include "properties/property.h"
#include <iostream>

void android::Property::Init() {
    android::PropInfo::Init();
    android::PropArea::Init();
    android::Propbt::Init();
}

std::string android::Property::Get(const char *name) {
    return Get(name, "");
}

std::string android::Property::Get(const char *name, const char* def) {
    android::PropInfo result = 0x0;
    auto callback = [name, def, &result](LoadBlock *block) -> bool {
        if (block->size() >= android::PropArea::PA_SIZE
                && !(block->size() % android::PropArea::PA_SIZE)) {
            android::PropArea area = block->vaddr();
            android::PropInfo info = area.find(name);

            if (info.Ptr()) {
                result = info;
                return true;
            }
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback);

    if (result.Ptr()) {
        return reinterpret_cast<const char*>(result.Real() + OFFSET(PropInfo, value));
    }
    return def;
}

void android::Property::Foreach(std::function<void (android::PropInfo& info)> propfn) {
    auto callback = [&propfn](LoadBlock *block) -> bool {
        if (block->size() >= android::PropArea::PA_SIZE
                && !(block->size() % android::PropArea::PA_SIZE)) {
            android::PropArea area = block->vaddr();
            area.foreach(propfn);
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback);
}
