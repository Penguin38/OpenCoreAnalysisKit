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
#include <fcntl.h>
#include <iostream>
#include <sstream>

bool Utils::SearchFile(const std::string& directory, std::string* result, const char* name) {
    if (!directory.empty() && name && name[0] != '\0') {
        return InnerSearchFile(directory, result, name);
    }
    return false;
}

bool Utils::InnerSearchFile(const std::string& directory, std::string* result, const std::string& name) {
    struct stat d_stat;
    stat(directory.c_str(), &d_stat);

    if (S_ISDIR(d_stat.st_mode)) {
        ListFiles(directory, result, name);
    } else if (S_ISREG(d_stat.st_mode)) {
        std::filesystem::path file(name);
        std::filesystem::path outer(directory);
        if (file.filename() == outer.filename()) {
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
        LOGD("Cannot opendir %s\n", directory.c_str());
        return;
    }

    struct dirent* dp;
    while ((dp = readdir(dirp)) != nullptr) {
        std::string filename(dp->d_name);

        if (filename == "." || filename == "..") {
            continue;
        }

        std::string file_path = directory;
        std::size_t last = file_path.find("/", file_path.length() - 1, 1);
        if (last == std::string::npos)
            file_path.append("/");
        file_path.append(filename);

        if (dp->d_type == DT_DIR) {
            ListFiles(file_path, result, name);
        } else {
            std::size_t index = file_path.find(name);
            if (index != std::string::npos) {
                std::filesystem::path file(name);
                std::filesystem::path outer(file_path);
                if (file.filename() == outer.filename()) {
                    result->append(file_path);
                    break;
                }
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

int Utils::FreopenWrite(const char* path) {
    int fd = dup(fileno(stdout));
    int out = open(path, O_CREAT | O_RDWR | O_TRUNC, 0666);
    dup2(out, fileno(stdout));
    close(out);
    return fd;
}

void Utils::CloseWriteout(int fd) {
    fflush(stdout);
    dup2(fd, fileno(stdout));
    close(fd);
}

std::string Utils::ToHex(uint64_t value) {
    std::string sb;
    std::ostringstream ss;
    ss << "0x" << std::hex << value;
    sb.append(ss.str());
    return sb;
}

uint32_t Utils::CRC32(uint8_t* data, uint32_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t k = 0; k < len; ++k) {
        crc ^= (uint32_t)data[k] << 24;
        for (int i = 0; i < 8; ++i) {
            if (crc & (1 << 31)) {
                crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc <<= 1;
            }
            crc &= 0xFFFFFFFF;
        }
    }
    return crc;
}

uint64_t Utils::CRC64(uint8_t* data, uint64_t len) {
    uint64_t crc = 0xFFFFFFFFFFFFFFFF;
    for (uint64_t k = 0; k < len; ++k) {
        crc ^= (uint64_t)data[k] << 56;
        for (int i = 0; i < 8; ++i) {
            if (crc & (1ULL << 63)) {
                crc = (crc << 1) ^ 0x42F0E1EBA9EA3693;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
