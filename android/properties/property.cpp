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
#include "base/utils.h"
#include "common/exception.h"
#include "properties/prop_area.h"
#include "properties/property.h"
#include <string.h>
#include <iostream>

void android::Property::Init() {
    android::PropInfo::Init();
    android::PropArea::Init();
    android::Propbt::Init();
}

const char* android::Property::Get(const char *name) {
    return Get(name, "");
}

const char* android::Property::Get(const char *name, const char* def) {
    android::PropInfo result = 0x0;
    auto callback = [name, def, &result](LoadBlock *block) -> bool {
        if (block->realSize() >= android::PropArea::PA_SIZE
                && block->realSize() <= android::PropArea::LARGE_PA_SIZE
                && !(block->realSize() % android::PropArea::PA_SIZE)) {
            try {
                android::PropArea area(block->vaddr(), block);
                android::PropInfo info = area.find(name);

                if (info.Ptr()) {
                    result = info;
                    return true;
                }
            } catch (InvalidAddressException& e) {
                // do nothing
            }
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, true, true);

    if (result.Ptr())
        return result.value();
    return def;
}

int64_t android::Property::GetInt64(const char *name) {
    return GetInt64(name, 0L);
}

int64_t android::Property::GetInt64(const char *name, int64_t def) {
    const char* value = Get(name);
    if (strcmp(value, ""))
        return atol(value);
    return def;
}

int32_t android::Property::GetInt32(const char *name) {
    return GetInt32(name, 0);
}

int32_t android::Property::GetInt32(const char *name, int32_t def) {
    const char* value = Get(name);
    if (strcmp(value, ""))
        return std::atoi(value);
    return def;
}

void android::Property::Foreach(std::function<void (android::PropInfo& info)> propfn) {
    auto callback = [&propfn](LoadBlock *block) -> bool {
        if (block->realSize() >= android::PropArea::PA_SIZE
                && block->realSize() <= android::PropArea::LARGE_PA_SIZE
                && !(block->realSize() % android::PropArea::PA_SIZE)) {
            android::PropArea area(block->vaddr(), block);
            area.foreach(propfn);
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, true, true);
}
