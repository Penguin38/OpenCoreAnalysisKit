/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_LOGCAT_LOG_H_
#define ANDROID_LOGCAT_LOG_H_

#include "logcat/SerializedLogBuffer.h"
#include <string>

namespace android {

typedef enum android_LogPriority {
    /** For internal use only.  */
    ANDROID_LOG_UNKNOWN = 0,
    /** The default priority, for internal use only.  */
    ANDROID_LOG_DEFAULT, /* only for SetMinPriority() */
    /** Verbose logging. Should typically be disabled for a release apk. */
    ANDROID_LOG_VERBOSE,
    /** Debug logging. Should typically be disabled for a release apk. */
    ANDROID_LOG_DEBUG,
    /** Informational logging. Should typically be disabled for a release apk. */
    ANDROID_LOG_INFO,
    /** Warning logging. For use with recoverable failures. */
    ANDROID_LOG_WARN,
    /** Error logging. For use with unrecoverable failures. */
    ANDROID_LOG_ERROR,
    /** Fatal logging. For use when aborting. */
    ANDROID_LOG_FATAL,
    /** For internal use only.  */
    ANDROID_LOG_SILENT, /* only for SetMinPriority(); must be last */
} android_LogPriority;

typedef enum log_id {
    LOG_ID_MIN = 0,

    /** The main log buffer. This is the only log buffer available to apps. */
    LOG_ID_MAIN = 0,
    /** The radio log buffer. */
    LOG_ID_RADIO = 1,
    /** The event log buffer. */
    LOG_ID_EVENTS = 2,
    /** The system log buffer. */
    LOG_ID_SYSTEM = 3,
    /** The crash log buffer. */
    LOG_ID_CRASH = 4,
    /** The statistics log buffer. */
    LOG_ID_STATS = 5,
    /** The security log buffer. */
    LOG_ID_SECURITY = 6,
    /** The kernel log buffer. */
    LOG_ID_KERNEL = 7,

    LOG_ID_MAX,

    /** Let the logging function choose the best log target. */
    LOG_ID_DEFAULT = 0x7FFFFFFF
} log_id_t;

inline std::string ConvertPriority(uint8_t prio) {
    switch (prio) {
        case ANDROID_LOG_VERBOSE: return "V";
        case ANDROID_LOG_DEBUG: return "D";
        case ANDROID_LOG_INFO: return "I";
        case ANDROID_LOG_WARN: return "W";
        case ANDROID_LOG_ERROR: return "E";
        case ANDROID_LOG_FATAL: return "F";
        case ANDROID_LOG_SILENT: return "S";
    }
    return "I";
}

class Logcat {
public:
    static void Init();
    static SerializedLogBuffer AnalysisSerializedLogBuffer();
};

} // namespace android

#endif // ANDROID_LOGCAT_LOG_H_
