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
#include <inttypes.h>
#include <sys/types.h>

#define ENTER() LOGI("\n");

#define ANSI_HIGH_LIGHT
#ifdef ANSI_HIGH_LIGHT
#define ANSI_COLOR_BLACK         "\033[30m"
#define ANSI_COLOR_RED           "\033[31m"
#define ANSI_COLOR_GREEN         "\033[32m"
#define ANSI_COLOR_YELLOW        "\033[33m"
#define ANSI_COLOR_BLUE          "\033[34m"
#define ANSI_COLOR_MAGENTA       "\033[35m"
#define ANSI_COLOR_CYAN          "\033[36m"
#define ANSI_COLOR_WHITE         "\033[37m"
#define ANSI_COLOR_LIGHTBLACK    "\033[90m"
#define ANSI_COLOR_LIGHTRED      "\033[91m"
#define ANSI_COLOR_LIGHTGREEN    "\033[92m"
#define ANSI_COLOR_LIGHTYELLOW   "\033[93m"
#define ANSI_COLOR_LIGHTBLUE     "\033[94m"
#define ANSI_COLOR_LIGHTMAGENTA  "\033[95m"
#define ANSI_COLOR_LIGHTCYAN     "\033[96m"
#define ANSI_COLOR_LIGHTWHITE    "\033[97m"
#define ANSI_COLOR_RESET         "\x1b[0m"
#else
#define ANSI_COLOR_BLACK         ""
#define ANSI_COLOR_RED           ""
#define ANSI_COLOR_GREEN         ""
#define ANSI_COLOR_YELLOW        ""
#define ANSI_COLOR_BLUE          ""
#define ANSI_COLOR_MAGENTA       ""
#define ANSI_COLOR_CYAN          ""
#define ANSI_COLOR_WHITE         ""
#define ANSI_COLOR_LIGHTBLACK    ""
#define ANSI_COLOR_LIGHTRED      ""
#define ANSI_COLOR_LIGHTGREEN    ""
#define ANSI_COLOR_LIGHTYELLOW   ""
#define ANSI_COLOR_LIGHTBLUE     ""
#define ANSI_COLOR_LIGHTMAGENTA  ""
#define ANSI_COLOR_LIGHTCYAN     ""
#define ANSI_COLOR_LIGHTWHITE    ""
#define ANSI_COLOR_RESET         ""
#endif
#define ANSI_COLOR_NONE          ""

#define LOG_DEBUG_PREFIX "DEBUG: "
#define LOG_WARN_PREFIX "WARN: "
#define LOG_ERROR_PREFIX "ERROR: "
#define LOG_FATAL_PREFIX "FATAL: "

#define LOGD2(...) \
do { \
    Logger::debug(2, __VA_ARGS__); \
} while(0)

#define LOGD(...) \
do { \
    Logger::debug(1, __VA_ARGS__); \
} while(0)

#define LOGI(...) \
do { \
    Logger::info(__VA_ARGS__); \
} while(0)

#define LOGW(...) \
do { \
    Logger::warn(__VA_ARGS__); \
} while(0)

#define LOGE(...) \
do { \
    Logger::error(__VA_ARGS__); \
} while(0)

#define LOGF(...) \
do { \
    Logger::fatal(__VA_ARGS__); \
} while(0)

class Logger {
public:
    constexpr static uint32_t LEVEL_DEBUG_2 = 2;
    constexpr static uint32_t LEVEL_DEBUG_1 = 1;
    constexpr static uint32_t LEVEL_NONE =    0;
    constexpr static uint32_t LEVEL_INFO =    1;
    constexpr static uint32_t LEVEL_WARN =    2;
    constexpr static uint32_t LEVEL_ERROR =   3;
    constexpr static uint32_t LEVEL_FATAL =   4;
    static bool IsDebug() { return Logger::GetDebugLevel(); }
    static uint32_t GetDebugLevel() { return INSTANCE->getDebugLevel(); }
    static void SetDebugLevel(int lv) { INSTANCE->setDebugLevel(lv); }
    static uint32_t GetLevel() { return INSTANCE->getLevel(); }
    static void SetLevel(int lv) { INSTANCE->setLevel(lv); }
    static bool IsLight() { return INSTANCE->isLight(); }
    static void SetHighLight(bool enable) { INSTANCE->setHighLight(enable); }

    static const char* LightBlack() { return IsLight() ? ANSI_COLOR_LIGHTBLACK : ANSI_COLOR_NONE; }
    static const char* Black() { return IsLight() ? ANSI_COLOR_BLACK : ANSI_COLOR_NONE; }
    static const char* LightRed() { return IsLight() ? ANSI_COLOR_LIGHTRED : ANSI_COLOR_NONE; }
    static const char* Red() { return IsLight() ? ANSI_COLOR_RED : ANSI_COLOR_NONE; }
    static const char* LightGreen() { return IsLight() ? ANSI_COLOR_LIGHTGREEN : ANSI_COLOR_NONE; }
    static const char* Green() { return IsLight() ? ANSI_COLOR_GREEN : ANSI_COLOR_NONE; }
    static const char* LightYellow() { return IsLight() ? ANSI_COLOR_LIGHTYELLOW : ANSI_COLOR_NONE; }
    static const char* Yellow() { return IsLight() ? ANSI_COLOR_YELLOW : ANSI_COLOR_NONE; }
    static const char* LightBlue() { return IsLight() ? ANSI_COLOR_LIGHTBLUE : ANSI_COLOR_NONE; }
    static const char* Blue() { return IsLight() ? ANSI_COLOR_BLUE : ANSI_COLOR_NONE; }
    static const char* LightMagenta() { return IsLight() ? ANSI_COLOR_LIGHTMAGENTA : ANSI_COLOR_NONE; }
    static const char* Magenta() { return IsLight() ? ANSI_COLOR_MAGENTA : ANSI_COLOR_NONE; }
    static const char* LightCyan() { return IsLight() ? ANSI_COLOR_LIGHTCYAN : ANSI_COLOR_NONE; }
    static const char* Cyan() { return IsLight() ? ANSI_COLOR_CYAN : ANSI_COLOR_NONE; }
    static const char* LightWhite() { return IsLight() ? ANSI_COLOR_LIGHTWHITE : ANSI_COLOR_NONE; }
    static const char* White() { return IsLight() ? ANSI_COLOR_WHITE : ANSI_COLOR_NONE; }
    static const char* End() { return IsLight() ? ANSI_COLOR_RESET : ANSI_COLOR_NONE; }
    Logger(int lv, int dv, bool light) :
        mLevel(lv), mDebug(dv), mHighLight(light) {}

    static void debug(uint32_t __lv, const char *__restrict __format, ...);
    static void info(const char *__restrict __format, ...);
    static void warn(const char *__restrict __format, ...);
    static void error(const char *__restrict __format, ...);
    static void fatal(const char *__restrict __format, ...);
private:
    inline uint32_t getDebugLevel() { return mDebug; }
    inline void setDebugLevel(int lv) { mDebug = lv; }
    inline uint32_t getLevel() { return mLevel; }
    inline void setLevel(int lv) { mLevel = lv; }
    inline bool isLight() { return mHighLight; }
    inline void setHighLight(int light) { mHighLight = light; }
    static Logger* INSTANCE;
    uint32_t mDebug;
    uint32_t mLevel;
    bool mHighLight;
};

#endif // UTILS_LOGGER_LOG_H_
