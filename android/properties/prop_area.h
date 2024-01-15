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

#ifndef ANDROID_PROPERTIES_PROP_AREA_H_
#define ANDROID_PROPERTIES_PROP_AREA_H_

#include "properties/prop_info.h"
#include <functional>

#define PROP_FILENAME "/dev/__properties__"
#define PROP_FILENAME_LEN 19

struct PropArea_OffsetTable {
    uint32_t bytes_used_;
    uint32_t serial_;
    uint32_t magic_;
    uint32_t version_;
    uint32_t data_;
};

struct PropArea_SizeTable {
    uint32_t THIS;
    uint32_t bytes_used_;
    uint32_t serial_;
    uint32_t magic_;
    uint32_t version_;
    uint32_t data_;
};

extern struct PropArea_OffsetTable __PropArea_offset__;
extern struct PropArea_SizeTable __PropArea_size__;

struct Propbt_OffsetTable {
    uint32_t namelen;
    uint32_t prop;
    uint32_t left;
    uint32_t right;
    uint32_t children;
    uint32_t name;
};

struct Propbt_SizeTable {
    uint32_t THIS;
    uint32_t namelen;
    uint32_t prop;
    uint32_t left;
    uint32_t right;
    uint32_t children;
    uint32_t name;
};

extern struct Propbt_OffsetTable __Propbt_offset__;
extern struct Propbt_SizeTable __Propbt_size__;

namespace android {
class Propbt : public api::MemoryRef {
public:
    Propbt(uint64_t v) : MemoryRef(v) {}
    template<typename U> Propbt(U *v) : MemoryRef(v) {}
    template<typename U> Propbt& operator=(U* other) { init(other); return *this; }
    static void Init();

    inline uint32_t namelen() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Propbt, namelen)); }
    inline uint32_t prop() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Propbt, prop)); }
    inline uint32_t left() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Propbt, left)); }
    inline uint32_t right() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Propbt, right)); }
    inline uint32_t children() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(Propbt, children)); }
    inline char* name() { return reinterpret_cast<char *>(Real() + OFFSET(Propbt, name)); }
};

class PropArea : public api::MemoryRef {
public:
    PropArea(uint64_t v) : MemoryRef(v) {}
    template<typename U> PropArea(U *v) : MemoryRef(v) {}
    template<typename U> PropArea& operator=(U* other) { init(other); return *this; }

    constexpr static uint32_t PA_SIZE = 128 * 1024;
    constexpr static uint32_t PROP_AREA_MAGIC = 0x504f5250;
    constexpr static uint32_t PROP_AREA_VERSION = 0xfc6ed0ab;

    static void Init();
    inline uint32_t magic() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(PropArea, magic_)); }
    inline uint32_t version() { return *reinterpret_cast<uint32_t *>(Real() + OFFSET(PropArea, version_)); }
    inline char* data() { return reinterpret_cast<char*>(Ptr() + OFFSET(PropArea, data_)); }

    void* toPropObj(uint32_t off);
    Propbt toPropbt(uint32_t off);
    PropInfo toPropInfo(uint32_t off);
    Propbt rootNode();

    PropInfo findProperty(Propbt& trie, const char* name);
    Propbt findPropbt(Propbt& bt, const char* name, uint32_t namelen);
    PropInfo find(const char* name);
    bool foreachProperty(Propbt& trie, std::function<void (PropInfo& pi)> propfn);
    bool foreach(std::function<void (PropInfo& pi)> propfn);
};
} // android

#endif // ANDROID_PROPERTIES_PROP_AREA_H_
