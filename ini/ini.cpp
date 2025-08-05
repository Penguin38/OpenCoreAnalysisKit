/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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
#include "ini.h"
#include <stdio.h>
#include <string.h>

bool Ini::LoadFile(const char* path) {
    char line[1024];
    std::string section_name;
    FILE *fp = fopen(path, "r");
    if (!fp) return false;

    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#')
            continue;

        if (line[0] == '[') {
            int tail = strlen(line) - 2;
            if (tail <= 1) {
                // [],[
                fclose(fp);
                return false;
            }

            if (line[tail] != ']') {
                fclose(fp);
                return false;
            }

            section_name = line;
            section_name = section_name.substr(1, tail - 1);

            auto it = sections.find(section_name);
            if (it == sections.end()) {
                std::unordered_map<std::string, std::string> entrys;
                sections.insert(std::make_pair(section_name, entrys));
            }
            continue;
        }

        if (section_name.length()) {
            std::string entry = line;
            if (entry.length() <= 2)
                continue;

            std::size_t l_pos = entry.find("=");
            if (l_pos != std::string::npos) {
                if (!(l_pos + 1 < entry.length()))
                    continue;

                std::string key = entry.substr(0, l_pos);
                std::string value = entry.substr(l_pos + 1, entry.length() - l_pos - 2);

                std::unordered_map<std::string, std::string>& entrys = sections[section_name];
                entrys.insert(std::make_pair(key, value));
            }
        }
    }
    fclose(fp);
    return true;
}

bool Ini::HasSection(const char* name) {
    auto it = sections.find(name);
    return it != sections.end();
}

const char* Ini::GetValue(const char* section, const char* key, const char* default_value) {
    auto it = sections.find(section);
    if (it == sections.end())
        return default_value;

    std::unordered_map<std::string, std::string>& entrys = it->second;
    auto entry = entrys.find(key);
    if (entry != entrys.end()) {
        std::string& value = entry->second;
        if (value.length())
            return value.c_str();
    }
    return default_value;
}
