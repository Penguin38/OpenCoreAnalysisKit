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

uint32_t android::PropArea::magic() {
    return *reinterpret_cast<uint32_t *>(Real() + OFFSET(PropArea, magic_));
}

uint32_t android::PropArea::version() {
    return *reinterpret_cast<uint32_t *>(Real() + OFFSET(PropArea, version_));
}

char* android::PropArea::data() {
    return reinterpret_cast<char*>(Ptr() + OFFSET(PropArea, data_));
}

void* android::PropArea::toPropObj(uint32_t off) {
    return data() + off;
}

inline android::Propbt* android::PropArea::toPropbt(uint32_t off) {
    return reinterpret_cast<android::Propbt*>(toPropObj(off));
}

inline android::PropInfo android::PropArea::toPropInfo(uint32_t off) {
    return toPropObj(off);
}

inline android::Propbt* android::PropArea::rootNode() {
    return reinterpret_cast<android::Propbt*>(toPropObj(0));
}

static int ComparePropName(const char* one, uint32_t one_len, const char* two, uint32_t two_len) {
    if (one_len < two_len)
        return -1;
    else if (one_len > two_len)
        return 1;
    else
        return strncmp(one, two, one_len);
}

android::Propbt* android::PropArea::findPropbt(android::Propbt* const bt, const char* name, uint32_t namelen) {
    android::Propbt* current = bt;
    while (true) {
        if (!current) {
            return nullptr;
        }
        android::Propbt* core_current = reinterpret_cast<android::Propbt*>(CoreApi::GetReal((uint64_t)current));

        const int ret = ComparePropName(name, namelen, core_current->name, core_current->namelen);
        if (ret == 0) {
            return current;
        }

        if (ret < 0) {
            uint32_t left_offset = core_current->left;
            if (left_offset != 0) {
                current = toPropbt(left_offset);
            } else {
                return nullptr;
            }
        } else {
            uint32_t right_offset = core_current->right;
            if (right_offset != 0) {
                current = toPropbt(right_offset);
            } else {
                return nullptr;
            }
        }
    }
}

android::PropInfo android::PropArea::findProperty(android::Propbt* const trie, const char* name) {
    if (!trie) return 0x0;

    const char* remaining_name = name;

    android::Propbt* current = trie;

    while(true) {
        android::Propbt* core_current = reinterpret_cast<android::Propbt*>(CoreApi::GetReal((uint64_t)current));

        const char* sep = strchr(remaining_name, '.');
        const bool want_subtree = (sep != nullptr);
        const uint32_t substr_size = (want_subtree) ? sep - remaining_name : strlen(remaining_name);

        if (!substr_size) {
            return 0x0;
        }

        android::Propbt* root = nullptr;
        uint32_t children_offset = core_current->children;

        if (children_offset != 0) {
            root = toPropbt(children_offset);
        }

        if (!root) {
            return 0x0;
        }

        current = findPropbt(root, remaining_name, substr_size);
        if (!current) {
            return 0x0;
        }

        if (!want_subtree) break;

        remaining_name = sep + 1;
    }

    android::Propbt* core_current = reinterpret_cast<android::Propbt*>(CoreApi::GetReal((uint64_t)current));
    uint32_t prop_offset = core_current->prop;
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
    return findProperty(rootNode(), name);
}

bool android::PropArea::foreachProperty(android::Propbt* const trie, std::function<void (android::PropInfo& pi)> propfn) {
    if (!trie) return false;
    android::Propbt* core_trie = reinterpret_cast<android::Propbt*>(CoreApi::GetReal((uint64_t)trie));

    uint32_t left_offset = core_trie->left;
    if (left_offset != 0) {
        const int err = foreachProperty(toPropbt(left_offset), propfn);
        if (err < 0) return false;
    }
    uint32_t prop_offset = core_trie->prop;
    if (prop_offset != 0) {
        android::PropInfo info = toPropInfo(prop_offset);
        if (!info.Ptr()) return false;
        if (propfn) propfn(info);
    }
    uint32_t children_offset = core_trie->children;
    if (children_offset != 0) {
        const int err = foreachProperty(toPropbt(children_offset), propfn);
        if (err < 0) return false;
    }
    uint32_t right_offset = core_trie->right;
    if (right_offset != 0) {
        const int err = foreachProperty(toPropbt(right_offset), propfn);
        if (err < 0) return false;
    }

    return true;
}

bool android::PropArea::foreach(std::function<void (android::PropInfo& pi)> propfn) {
    if (version() != PROP_AREA_VERSION || magic() != PROP_AREA_MAGIC) {
        return false;
    }
    return foreachProperty(rootNode(), propfn);
}
