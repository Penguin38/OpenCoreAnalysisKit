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

#ifndef ANDROID_PROPERTIES_PROP_INFO_H_
#define ANDROID_PROPERTIES_PROP_INFO_H_

#include "api/memory_ref.h"
#include <stdint.h>
#include <sys/types.h>
#include <string>

#define PROP_VALUE_MAX  92

struct PropInfo_OffsetTable {
    uint32_t serial;
    uint32_t value;
    uint32_t name;
};

struct PropInfo_SizeTable {
    uint32_t THIS;
    uint32_t serial;
    uint32_t value;
    uint32_t name;
};

extern struct PropInfo_OffsetTable __PropInfo_offset__;
extern struct PropInfo_SizeTable __PropInfo_size__;

namespace android {
class PropInfo : public api::MemoryRef {
public:
    PropInfo(uint64_t v) : MemoryRef(v) {}
    template<typename U> PropInfo(U *v) : MemoryRef(v) {}
    template<typename U> PropInfo& operator=(U* other) { init(other); return *this; }

    constexpr static uint32_t kLongFlag = 1 << 16;
    constexpr static uint32_t kLongLegacyErrorBufferSize = 56;

    static void Init();
    inline std::string name() { return reinterpret_cast<const char*>(Real() + OFFSET(PropInfo, name)); }
    inline std::string value() { return reinterpret_cast<const char*>(Real() + OFFSET(PropInfo, value)); }
};

} // android

#endif // ANDROID_PROPERTIES_PROP_INFO_H_
