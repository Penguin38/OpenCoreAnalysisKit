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

#ifndef PARSER_COMMAND_ANDROID_CMD_FORMAT_DUMP_H_
#define PARSER_COMMAND_ANDROID_CMD_FORMAT_DUMP_H_

#include "runtime/mirror/object.h"
#include "java/lang/Throwable.h"
#include "java/lang/String.h"
#include "android/util/ArrayMap.h"

typedef void (*FormatDumpCall)(const char* prefix, art::mirror::Object& obj);

struct ObjectFormatDump {
    const char* instance;
    FormatDumpCall call;
};

static ObjectFormatDump kFormatDump[] = {
    { "java.lang.Throwable", java::lang::Throwable::FormatDump },
    { "java.lang.String", java::lang::String::FormatDump },
    { "android.util.ArrayMap", android::util::ArrayMap<java::lang::Object, java::lang::Object>::FormatDump },
};

static FormatDumpCall GetFormatDumpCall(art::mirror::Object& obj) {
    java::lang::Object instance = obj;
    int count = sizeof(kFormatDump)/sizeof(kFormatDump[0]);
    for (int index = 0; index < count; ++index) {
        if (instance.instanceof(kFormatDump[index].instance)) {
            return kFormatDump[index].call;
        }
    }
    return nullptr;
}

#endif // PARSER_COMMAND_ANDROID_CMD_FORMAT_DUMP_H_
