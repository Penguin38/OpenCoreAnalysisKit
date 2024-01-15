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

#ifndef UTILS_LOGGER_LOG_H_
#define UTILS_LOGGER_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#define LOGI(...) \
do { \
    printf(__VA_ARGS__); \
} while(0)

#define LOGD(...) \
do { \
    if (Logger::GetLevel() > Logger::LEVEL_INFO) \
        printf(__VA_ARGS__); \
} while(0)

#define LOGW(...) \
do { \
    if (Logger::GetLevel() > Logger::LEVEL_DEBUG) \
        printf(__VA_ARGS__); \
} while(0)

#define LOGE(...) \
do { \
    if (Logger::GetLevel() > Logger::LEVEL_WARN) \
        printf(__VA_ARGS__); \
} while(0)

#define LOGF(...) \
do { \
    if (Logger::GetLevel() > Logger::LEVEL_ERROR) \
        printf(__VA_ARGS__); \
} while(0)

class Logger {
public:
    constexpr static uint32_t LEVEL_INFO =    0;
    constexpr static uint32_t LEVEL_DEBUG =   1;
    constexpr static uint32_t LEVEL_WARN =    2;
    constexpr static uint32_t LEVEL_ERROR =   3;
    constexpr static uint32_t LEVEL_FATAL =   4;
    static uint32_t GetLevel() { return INSTANCE->getLevel(); }
    static void SetLevel(int lv) { INSTANCE->setLevel(lv); }
    Logger(int lv) { mLevel = lv; }
private:
    inline uint32_t getLevel() { return mLevel; }
    inline void setLevel(int lv) { mLevel = lv; }
    static Logger* INSTANCE;
    uint32_t mLevel;
};

#endif // UTILS_LOGGER_LOG_H_
