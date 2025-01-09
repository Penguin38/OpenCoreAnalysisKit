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

#include "command/remote/setprop/property.h"
#include "command/remote/setprop/prop_info.h"
#include "command/remote/setprop/prop_area.h"
#include "command/remote/opencore/opencore.h"
#include "command/remote/cmd_remote.h"
#include "base/memory_map.h"
#include "logger/log.h"
#include "api/core.h"
#include <dirent.h>
#include <stdio.h>
#include <string>
#include <memory>

int AndroidProperty::Main(int argc, char* const argv[]) {
    if (argc != 3) {
        AndroidProperty::Usage();
        return -1;
    }

    std::string name = argv[1];
    std::string value = argv[2];

    if (name.front() == '.' || name.back() == '.') {
        LOGE("Property names must not start or end with a '.'\n");
        return -1;
    }

    if (name.find("..") != std::string::npos) {
        LOGE("'..' is not allowed in a property name\n");
        return -1;
    }

    for (const auto& c : name) {
        if (!isalnum(c) && !strchr(":@_.-", c)) {
            LOGE("Invalid character '%c' in name '%s'\n", c, name.c_str());
            return -1;
        }
    }

    if (value.size() >= PROP_VALUE_MAX && !(value.substr(0,3) == std::string{"ro."})) {
        LOGE("Value '%s' is too long, %ld bytes vs a max of %d\n", value.c_str(), value.size(), PROP_VALUE_MAX);
        return -1;
    }

    if (mbstowcs(nullptr, value.data(), 0) == static_cast<std::size_t>(-1)) {
        LOGE("Value '%s' is not a UTF8 encoded string\n", value.c_str());
        return -1;
    }

    return Set(name, value);
}

int AndroidProperty::Set(std::string& name, std::string& value) {
    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(1);

    struct dirent *entry;
    char line[1024];
    uint64_t prop_info_ptr = 0x0;
    const char* new_value = value.c_str();

    DIR *dp = opendir(PROP_FILENAME);
    if (dp) {
        while((entry=readdir(dp)) != NULL) {
            if(!strncmp(entry->d_name, ".", 1))
                continue;

            char entry_path[256] = {'\0'};
            snprintf(entry_path, sizeof(entry_path), "%s/%s", PROP_FILENAME, entry->d_name);

            std::unique_ptr<MemoryMap> map(MemoryMap::MmapFile(entry_path));
            if (map) {
                prop_area* area = (prop_area *)map->data();
                prop_info* info = area->find(name.c_str());

                if (info) {
                    uint32_t offset = ((uint64_t)info)-map->data();
                    FILE *fp = fopen("/proc/1/maps", "r");
                    if (!fp) return -1;

                    while (fgets(line, sizeof(line), fp)) {
                        int m = -1;
                        void *start;
                        char filename[256];

                        sscanf(line, "%p-%*p %*c%*c%*c%*c %*x %*x:%*x  %*u %[^\n] %n", &start, filename, &m);
                        if (!strncmp(entry_path, filename, strlen(entry_path))) {
                            prop_info_ptr = (uint64_t)start + offset;
                            break;
                        }
                    }
                    fclose(fp);
                    break;
                }
            }
        }
        closedir(dp);
    }

    if (!prop_info_ptr) return -1;

    prop_info new_info;
    if (!RemoteCommand::Read(1, prop_info_ptr, sizeof(prop_info), (uint8_t *)&new_info))
        return -1;

    uint32_t count = (new_info.serial >> 24);
    memset(&new_info.value, 0x0, count);

    new_info.serial &= 0xFFFFFF;
    new_info.serial |= (strlen(new_value) << 24);
    memcpy(&new_info.value, new_value, strlen(new_value));

    RemoteCommand::Write(1, prop_info_ptr, (void *)&new_info, sizeof(prop_info));
    return 0;
}

void AndroidProperty::Usage() {
    LOGI("Usage: remote setprop <NAME> <VALUE>\n");
}
