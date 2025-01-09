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

#ifndef PARPSER_COMMAND_REMOTE_SETPROP_PROP_AREA_H_
#define PARPSER_COMMAND_REMOTE_SETPROP_PROP_AREA_H_

#include "command/remote/setprop/prop_info.h"

#define PROP_FILENAME      "/dev/__properties__"
#define PROP_FILENAME_LEN  19

class prop_bt {
public:
    uint32_t namelen;
    uint32_t prop;
    uint32_t left;
    uint32_t right;
    uint32_t children;
    char name[0];
};

class prop_area {
public:
    uint32_t bytes_used_;
    uint32_t serial_;
    uint32_t magic_;
    uint32_t version_;
    uint32_t reserved_[28];
    char data_[0];

    void* to_prop_obj(uint32_t off);
    prop_bt* to_prop_bt(uint32_t* off_p);
    prop_info* to_prop_info(uint32_t* off_p);
    prop_bt* root_node();

    prop_info* find_property(prop_bt* const trie, const char* name);
    prop_bt* find_prop_bt(prop_bt* const bt, const char* name, uint32_t namelen);
    prop_info* find(const char* name);
};

#endif // PARPSER_COMMAND_REMOTE_SETPROP_PROP_AREA_H_
