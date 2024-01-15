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
#include "base/utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <inttypes.h>
#include <filesystem>
#include <iostream>

bool Utils::SearchFile(const std::string& directory, std::string* result, const std::string& name) {
    struct stat d_stat;
    stat(directory.c_str(), &d_stat);

    if (S_ISDIR(d_stat.st_mode)) {
        ListFiles(directory, result, name);
    } else if (S_ISREG(d_stat.st_mode)) {
        std::filesystem::path file(name);
        std::size_t index = directory.find(file.filename());
        if (index != std::string::npos) {
            result->append(directory);
        }
    }

    if (result->length() > 0)
        return true;
    return false;
}

void Utils::ListFiles(const std::string& directory, std::string* result, const std::string& name) {
    DIR* dirp = opendir(directory.c_str());
    if (dirp == nullptr) {
        LOGI("Cannot opendir %s\n", directory.c_str());
        return;
    }

    struct dirent* dp;
    while ((dp = readdir(dirp)) != nullptr) {
        std::string filename(dp->d_name);

        if (filename == "." || filename == "..") {
            continue;
        }

        if (dp->d_type == DT_DIR) {
            ListFiles(directory + "/" + filename, result, name);
        } else {
            std::string file_path = directory + "/" + filename;
            std::size_t index = file_path.find(name);
            if (index != std::string::npos) {
                result->append(file_path);
                break;
            }
        }

        if (result->length() > 0)
            break;
    }

    closedir(dirp);
}

std::string Utils::ConvertAscii(uint64_t value, int len) {
    std::string sb;
    uint8_t byte;
    for (int i = 0; i < len; i++) {
        byte = (value >> (8 * i)) & 0xFF;
        if (byte > 0x20 && byte < 0x7F) {
            sb += byte;
        } else {
            sb.append(".");
        }
    }
    return sb;
}

uint64_t Utils::atol(const char* src) {
    uint64_t value;
    sscanf(src, "%" PRIx64 "", &value);
    return value;
}

void Utils::FreopenWrite(const char* path) {
    freopen(path, "w", stdout);
}

void Utils::CloseWriteout() {
    dup2(fileno(stdin), 1);
}