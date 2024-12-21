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
#include <stdarg.h>
#include <string>

Logger gLog(Logger::LEVEL_ERROR, Logger::LEVEL_NONE, true);
Logger* Logger::INSTANCE = &gLog;

static void FilterAnsiColor(std::string& __format__) {
#ifdef ANSI_HIGH_LIGHT
    if (!Logger::IsLight()) {
        std::size_t pos = __format__.find("\033[3");
        if (pos != std::string::npos) {
            do {
                __format__.erase(pos, 5);
                pos = __format__.find("\033[3");
            } while (pos != std::string::npos);
        }

        pos = __format__.find("\033[9");
        if (pos != std::string::npos) {
            do {
                __format__.erase(pos, 5);
                pos = __format__.find("\033[9");
            } while (pos != std::string::npos);
        }

        pos = __format__.find("\x1b[0m");
        if (pos != std::string::npos) {
            do {
                __format__.erase(pos, 4);
                pos = __format__.find("\x1b[0m");
            } while (pos != std::string::npos);
        }
    }
#endif
}

void Logger::debug(uint32_t __lv, const char *__restrict __format, ...) {
    if (Logger::GetDebugLevel() >= __lv) {
        std::string buffer;
        buffer.append(Logger::LightGreen());
        buffer.append(__format);
        buffer.append(Logger::End());
        FilterAnsiColor(buffer);
        va_list ap;
        va_start(ap, __format);
        vfprintf(stdout, buffer.c_str(), ap);
        va_end(ap);
    }
}

void Logger::info(const char *__restrict __format, ...) {
    // always output log
    std::string __format__(__format);
    FilterAnsiColor(__format__);
    va_list ap;
    va_start(ap, __format);
    vfprintf(stdout, __format__.c_str(), ap);
    va_end(ap);
}

void Logger::warn(const char *__restrict __format, ...) {
    if (Logger::GetLevel() >= Logger::LEVEL_WARN) {
        std::string buffer;
        buffer.append(Logger::LightYellow());
        buffer.append(LOG_WARN_PREFIX);
        buffer.append(__format);
        buffer.append(Logger::End());
        FilterAnsiColor(buffer);
        va_list ap;
        va_start(ap, __format);
        vfprintf(stdout, buffer.c_str(), ap);
        va_end(ap);
    }
}

void Logger::error(const char *__restrict __format, ...) {
    if (Logger::GetLevel() >= Logger::LEVEL_ERROR) {
        std::string buffer;
        buffer.append(Logger::LightRed());
        buffer.append(LOG_ERROR_PREFIX);
        buffer.append(__format);
        buffer.append(Logger::End());
        FilterAnsiColor(buffer);
        va_list ap;
        va_start(ap, __format);
        vfprintf(stdout, buffer.c_str(), ap);
        va_end(ap);
    }
}

void Logger::fatal(const char *__restrict __format, ...) {
    if (Logger::GetLevel() >= Logger::LEVEL_FATAL) {
        std::string buffer;
        buffer.append(Logger::Red());
        buffer.append(LOG_FATAL_PREFIX);
        buffer.append(__format);
        buffer.append(Logger::End());
        FilterAnsiColor(buffer);
        va_list ap;
        va_start(ap, __format);
        vfprintf(stdout, buffer.c_str(), ap);
        va_end(ap);
    }
}
