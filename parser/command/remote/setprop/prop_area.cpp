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

#include "logger/log.h"
#include "command/remote/setprop/prop_area.h"
#include "properties/prop_area.h"
#include <string.h>

void* prop_area::to_prop_obj(uint32_t off) {
    return (data_ + off);
}

inline prop_bt* prop_area::to_prop_bt(uint32_t* off_p) {
    uint32_t off = *off_p;
    return reinterpret_cast<prop_bt *>(to_prop_obj(off));
}

inline prop_info* prop_area::to_prop_info(uint32_t* off_p) {
    uint32_t off = *off_p;
    return reinterpret_cast<prop_info *>(to_prop_obj(off));
}

inline prop_bt* prop_area::root_node() {
    return reinterpret_cast<prop_bt *>(to_prop_obj(0));
}

static int cmp_prop_name(const char* one, uint32_t one_len, const char* two, uint32_t two_len) {
    if (one_len < two_len)
        return -1;
    else if (one_len > two_len)
        return 1;
    else
        return strncmp(one, two, one_len);
}

prop_bt* prop_area::find_prop_bt(prop_bt* const bt, const char* name, uint32_t namelen) {
    prop_bt* current = bt;
    while (true) {
        if (!current) {
            return nullptr;
        }

        const int ret = cmp_prop_name(name, namelen, current->name, current->namelen);
        if (ret == 0) {
            return current;
        }

        if (ret < 0) {
            uint32_t left_offset = current->left;
            if (left_offset != 0) {
                current = to_prop_bt(&current->left);
            } else {
                return nullptr;
            }
        } else {
            uint32_t right_offset = current->right;
            if (right_offset != 0) {
                current = to_prop_bt(&current->right);
            } else {
                return nullptr;
            }
        }
    }
}

prop_info* prop_area::find_property(prop_bt* const trie, const char* name) {
    if (!trie) return nullptr;

    const char* remaining_name = name;

    prop_bt* current = trie;

    while(true) {
        const char* sep = strchr(remaining_name, '.');
        const bool want_subtree = (sep != nullptr);
        const uint32_t substr_size = (want_subtree) ? sep - remaining_name : strlen(remaining_name);

        if (!substr_size) {
            return nullptr;
        }

        prop_bt* root = nullptr;
        uint32_t children_offset = current->children;

        if (children_offset != 0) {
            root = to_prop_bt(&current->children);
        }

        if (!root) {
            return nullptr;
        }

        current = find_prop_bt(root, remaining_name, substr_size);
        if (!current) {
            return nullptr;
        }

        if (!want_subtree) break;

        remaining_name = sep + 1;
    }

    uint32_t prop_offset = current->prop;
    if (prop_offset != 0) {
        return to_prop_info(&current->prop);
    } else {
        return nullptr;
    }
}

prop_info* prop_area::find(const char* name) {
    if (version_ != android::PropArea::PROP_AREA_VERSION
            || magic_ != android::PropArea::PROP_AREA_MAGIC) {
        return nullptr;
    }
    return find_property(root_node(), name);
}
