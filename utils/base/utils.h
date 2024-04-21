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

#ifndef UTILS_BASE_UTILS_H_
#define UTILS_BASE_UTILS_H_

#include <string>

class Utils {
public:
    static bool SearchFile(const std::string& directory, std::string* result, const std::string& name);
    static void ListFiles(const std::string& directory, std::string* result, const std::string& name);
    static std::string ConvertAscii(uint64_t value, int len);
    static uint64_t atol(const char* src);
    static int FreopenWrite(const char* path);
    static void CloseWriteout(int fd);
    static std::string ToHex(uint64_t value);
};

#endif // UTILS_BASE_UTILS_H_
