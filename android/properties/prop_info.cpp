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

#include "properties/prop_info.h"

struct PropInfo_OffsetTable __PropInfo_offset__;
struct PropInfo_SizeTable __PropInfo_size__;

void android::PropInfo::Init() {
    __PropInfo_offset__ = {
        .serial = 0,
        .value = 4,
        .name = 96,
    };

    __PropInfo_size__ = {
        .THIS = 96,
        .serial = 4,
        .value = 92,
        .name = 0,
    };
}

std::string android::PropInfo::name() {
    return reinterpret_cast<const char*>(Real() + OFFSET(PropInfo, name));
}

std::string android::PropInfo::value() {
    return reinterpret_cast<const char*>(Real() + OFFSET(PropInfo, value));
}
