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

// (<name>|data type[|data unit])
// The data type is a number from the following values:
// 1: int
// 2: long
// 3: string
// 4: list

// The data unit is a number taken from the following list:
// 1: Number of objects
// 2: Number of bytes
// 3: Number of milliseconds
// 4: Number of allocations
// 5: Id
// 6: Percent
// s: Number of seconds (monotonic time)
// Default value for data of type int/long is 2 (bytes).

// 1003 auditd (avc|3)
// 1004 chatty (dropped|3)
// 1005 tag_def (tag|1),(name|3),(format|3)
// 30023 am_kill (User|1|5),(PID|1|5),(Process Name|3),(OomAdj|1|5),(Reason|3)
// 30053 am_uid_stopped (UID|1|5)
// 30047 am_pss (Pid|1|5),(UID|1|5),(Process Name|3),(Pss|2|2),(Uss|2|2),(SwapPss|2|2),(Rss|2|2),(StatType|1|5),(ProcState|1|5),(TimeToCollect|2|2)

#ifndef ANDROID_LOGCAT_EVENT_LOG_TAGS_H_
#define ANDROID_LOGCAT_EVENT_LOG_TAGS_H_

#include <string>

namespace android {

class EventTags {
public:
    enum DataType {
        TAGS_TYPE_INT,
        TAGS_TYPE_LONG,
        TAGS_TYPE_STRING,
        TAGS_TYPE_LIST,
    };

    static constexpr int kNumEvents = 534;

    int id;
    const char* tag;

    static std::string ConvertTags(int id);
};

extern EventTags kEventTags[EventTags::kNumEvents];

} // namespace android

#endif  // ANDROID_LOGCAT_EVENT_LOG_TAGS_H_
