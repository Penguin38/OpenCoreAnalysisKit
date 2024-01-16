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
#include <string.h>

struct PropArea_OffsetTable __PropArea_offset__;
struct PropArea_SizeTable __PropArea_size__;

void android::PropArea::Init() {
    __PropArea_offset__ = {
        .bytes_used_ = 0,
        .serial_ = 4,
        .magic_ = 8,
        .version_ = 12,
        .data_ = 128,
    };

    __PropArea_size__ = {
        .THIS = 128,
        .bytes_used_ = 4,
        .serial_ = 4,
        .magic_ = 4,
        .version_ = 4,
        .data_ = 0,
    };
}

struct Propbt_OffsetTable __Propbt_offset__;
struct Propbt_SizeTable __Propbt_size__;

void android::Propbt::Init() {
    __Propbt_offset__ = {
        .namelen = 0,
        .prop = 4,
        .left = 8,
        .right = 12,
        .children = 16,
        .name = 20,
    };

    __Propbt_size__ = {
        .THIS = 20,
        .namelen = 4,
        .prop = 4,
        .left = 4,
        .right = 4,
        .children = 4,
        .name = 0,
    };
}

void* android::PropArea::toPropObj(uint32_t off) {
    return data() + off;
}

inline android::Propbt android::PropArea::toPropbt(uint32_t off) {
    android::Propbt bt(toPropObj(off), this);
    return bt;
}

inline android::PropInfo android::PropArea::toPropInfo(uint32_t off) {
    android::PropInfo info(toPropObj(off), this);
    return info;
}

inline android::Propbt android::PropArea::rootNode() {
    return toPropbt(0);
}

static int ComparePropName(const char* one, uint32_t one_len, const char* two, uint32_t two_len) {
    if (one_len < two_len)
        return -1;
    else if (one_len > two_len)
        return 1;
    else
        return strncmp(one, two, one_len);
}

android::Propbt android::PropArea::findPropbt(android::Propbt& bt, const char* name, uint32_t namelen) {
    android::Propbt current = bt;
    while (true) {
        if (!current.Ptr()) {
            return 0x0;
        }

        const int ret = ComparePropName(name, namelen, current.name(), current.namelen());
        if (ret == 0) {
            return current;
        }

        if (ret < 0) {
            uint32_t left_offset = current.left();
            if (left_offset != 0) {
                current = toPropbt(left_offset);
            } else {
                return 0x0;
            }
        } else {
            uint32_t right_offset = current.right();
            if (right_offset != 0) {
                current = toPropbt(right_offset);
            } else {
                return 0x0;
            }
        }
    }
}

android::PropInfo android::PropArea::findProperty(android::Propbt& trie, const char* name) {
    if (!trie.Ptr()) return 0x0;

    const char* remaining_name = name;

    android::Propbt current = trie;

    while(true) {

        const char* sep = strchr(remaining_name, '.');
        const bool want_subtree = (sep != nullptr);
        const uint32_t substr_size = (want_subtree) ? sep - remaining_name : strlen(remaining_name);

        if (!substr_size) {
            return 0x0;
        }

        android::Propbt root = 0x0;
        uint32_t children_offset = current.children();

        if (children_offset != 0) {
            root = toPropbt(children_offset);
        }

        if (!root.Ptr()) {
            return 0x0;
        }

        current = findPropbt(root, remaining_name, substr_size);
        if (!current.Ptr()) {
            return 0x0;
        }

        if (!want_subtree) break;

        remaining_name = sep + 1;
    }

    uint32_t prop_offset = current.prop();
    if (prop_offset != 0) {
        return toPropInfo(prop_offset);
    } else {
        return 0x0;
    }
}

android::PropInfo android::PropArea::find(const char* name) {
    if (version() != PROP_AREA_VERSION || magic() != PROP_AREA_MAGIC) {
        return 0x0;
    }
    android::Propbt root = rootNode();
    return findProperty(root, name);
}

bool android::PropArea::foreachProperty(android::Propbt& trie, std::function<void (android::PropInfo& pi)> propfn) {
    if (!trie.Ptr()) return false;

    uint32_t left_offset = trie.left();
    if (left_offset != 0) {
        android::Propbt left_node = toPropbt(left_offset);
        const int err = foreachProperty(left_node, propfn);
        if (err < 0) return false;
    }
    uint32_t prop_offset = trie.prop();
    if (prop_offset != 0) {
        android::PropInfo info = toPropInfo(prop_offset);
        if (!info.Ptr()) return false;
        if (propfn) propfn(info);
    }
    uint32_t children_offset = trie.children();
    if (children_offset != 0) {
        android::Propbt children_node = toPropbt(children_offset);
        const int err = foreachProperty(children_node, propfn);
        if (err < 0) return false;
    }
    uint32_t right_offset = trie.right();
    if (right_offset != 0) {
        android::Propbt right_node = toPropbt(right_offset);
        const int err = foreachProperty(right_node, propfn);
        if (err < 0) return false;
    }

    return true;
}

bool android::PropArea::foreach(std::function<void (android::PropInfo& pi)> propfn) {
    if (version() != PROP_AREA_VERSION || magic() != PROP_AREA_MAGIC) {
        return false;
    }
    android::Propbt root = rootNode();
    return foreachProperty(root, propfn);
}
