/*
 * Copyright (C) 2026-present, Guanyou.Chen. All rights reserved.
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
#include <windows.h>
#undef THIS
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <filesystem>

bool Utils::SearchFile(const std::string& directory, std::string* result, const char* name) {
    if (!directory.empty() && name && name[0] != '\0') {
        return InnerSearchFile(directory, result, name);
    }
    return false;
}

bool Utils::InnerSearchFile(const std::string& directory, std::string* result, const std::string& name) {
    DWORD attrs = GetFileAttributesA(directory.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES)
        return false;

    if (attrs & FILE_ATTRIBUTE_DIRECTORY) {
        ListFiles(directory, result, name);
    } else {
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
    std::string pattern = directory;
    if (pattern.back() != '\\' && pattern.back() != '/')
        pattern.append("\\");
    pattern.append("*");

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) {
        LOGD("Cannot opendir %s\n", directory.c_str());
        return;
    }

    do {
        std::string filename(fd.cFileName);

        if (filename == "." || filename == "..") {
            continue;
        }

        std::string file_path = directory;
        if (file_path.back() != '\\' && file_path.back() != '/')
            file_path.append("\\");
        file_path.append(filename);

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
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
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

int Utils::FreopenWrite(const char* path) {
    int fd = _dup(_fileno(stdout));
    int out = _open(path, _O_CREAT | _O_RDWR | _O_TRUNC, 0666);
    _dup2(out, _fileno(stdout));
    _close(out);
    return fd;
}

void Utils::CloseWriteout(int fd) {
    fflush(stdout);
    _dup2(fd, _fileno(stdout));
    _close(fd);
}
