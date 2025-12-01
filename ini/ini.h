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

#ifndef INI_INI_H_
#define INI_INI_H_

#include <string>
#include <unordered_map>
#include <functional>

class Ini {
public:
    Ini() {}
    bool LoadFile(const char* path);
    bool HasSection(const char* name);
    void Foreach(const char* section, std::function<void (const char* key, const char* value)> callback);
    const char* GetValue(const char* section, const char* key, const char* default_value);
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sections;
};

#endif // INI_INI_H_
