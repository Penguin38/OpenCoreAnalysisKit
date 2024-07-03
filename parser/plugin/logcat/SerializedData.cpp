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

#include "logger/log.h"
#include "api/core.h"
#include "SerializedData.h"
#include "event_logtags.h"
#include "log.h"
#include <time.h>
#include <vector>
#include <memory>

struct SerializedData_OffsetTable __SerializedData_offset__;

namespace android {

void SerializedData::Init() {
    if (CoreApi::Bits() == 64) {
        __SerializedData_offset__ = {
            .data_ = 0,
            .size_ = 8,
        };
    } else {
        // do nothing
    }
}

void SerializedData::DecodeDump(int filter, int id) {
    api::MemoryRef data_ = data();
    for (int i = 0; i < size(); ++i) {
        SerializedData::SerializedElement element;
        element.uid = data_.value32Of();
        element.pid = data_.value32Of(4);
        element.tid = data_.value32Of(8);

        if (!element.pid)
            break;

        element.tv_sec = data_.value32Of(20);
        element.tv_nsec = data_.value32Of(24);

        element.msg_len = data_.value16Of(28);
        element.log_id = data_.value8Of(30);

        if (filter & FILTER_PID && element.pid != id) {
            data_.MovePtr(30 + element.msg_len);
            continue;
        }

        if (filter & FILTER_UID && element.uid != id) {
            data_.MovePtr(30 + element.msg_len);
            continue;
        }

        if (filter & FILTER_TID && element.tid != id) {
            data_.MovePtr(30 + element.msg_len);
            continue;
        }

        if (element.log_id > LOG_ID_MAX) {
            element.log_id = ANDROID_LOG_INFO;
            element.event_id = data_.value32Of(30);

            element.tag = EventTags::ConvertTags(element.event_id);

            uint8_t type = data_.value8Of(34);
            switch (type) {
                case EventTags::TAGS_TYPE_INT: {
                    element.msg.append(std::to_string(data_.value32Of(35)));
                } break;
                case EventTags::TAGS_TYPE_LONG: {
                    element.msg.append(std::to_string(data_.value64Of(35)));
                } break;
                case EventTags::TAGS_TYPE_STRING: {
                    uint32_t len = data_.value32Of(35);
                    std::string submsg = reinterpret_cast<const char*>(data_.Real() + 39);
                    submsg.resize(len);
                    element.msg.append(submsg);
                } break;
                case EventTags::TAGS_TYPE_LIST: {
                    uint8_t count = data_.value8Of(35);
                    element.msg.append("[");
                    api::MemoryRef subcontent = data_.Ptr() + 36;
                    for (int i = 0; i < count; i++) {
                        uint8_t subtype = subcontent.value8Of();
                        switch (subtype) {
                            case EventTags::TAGS_TYPE_INT: {
                                element.msg.append(std::to_string(subcontent.value32Of(1)));
                                subcontent.MovePtr(sizeof(uint8_t) + sizeof(uint32_t));
                            } break;
                            case EventTags::TAGS_TYPE_LONG: {
                                element.msg.append(std::to_string(subcontent.value64Of(1)));
                                subcontent.MovePtr(sizeof(uint8_t) + sizeof(uint64_t));
                            } break;
                            case EventTags::TAGS_TYPE_STRING: {
                                uint32_t len = subcontent.value32Of(1);
                                std::string msg2 = reinterpret_cast<const char*>(subcontent.Real() + 0x5);
                                msg2.resize(len);
                                element.msg.append(msg2);
                                subcontent.MovePtr(len + sizeof(uint8_t) + sizeof(uint32_t));
                            } break;
                        }

                        if (i != count - 1)
                            element.msg.append(",");
                    }
                    element.msg.append("]");
                } break;
            }
        } else {
            element.tag = reinterpret_cast<const char*>(data_.Real() + 31);
            element.msg = reinterpret_cast<const char*>(data_.Real() + 31 + element.tag.length() + 1);
        }

        std::string time;
        uint64_t timestamp = (uint64_t)element.tv_sec * 1000L + element.tv_nsec / 1000000;
        int ms = timestamp % 1000;
        time_t tick = (time_t)(timestamp / 1000);
        struct tm tm;
        char s[40];
        tm = *localtime(&tick);
        strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
        time.append(s);
        time.append(".");
        char value[4];
        sprintf(value, "%03d", ms);
        time.append(value);

        std::vector<char *> logs;
        std::unique_ptr<char> newmsg(strdup(element.msg.c_str()));
        char *token = strtok(newmsg.get(), "\n");
        while (token != nullptr) {
            LOGI("%s %6d %5d %5d %s %s: %s\n", time.c_str(), element.uid, element.pid, element.tid,
                                               ConvertPriority(element.log_id).c_str(), element.tag.c_str(), token);
            token = strtok(NULL, "\n");
        }
        data_.MovePtr(30 + element.msg_len);
    }
}

} // namespace android
