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

#ifndef PARSER_COMMAND_ANDROID_CMD_SEARCH_H_
#define PARSER_COMMAND_ANDROID_CMD_SEARCH_H_

#include "command/command.h"
#include "runtime/mirror/object.h"
#include "android.h"

class SearchCommand : public Command {
public:
    static constexpr int SEARCH_OBJECT = 1 << 0;
    static constexpr int SEARCH_CLASS = 1 << 1;

    SearchCommand() : Command("search") {}
    ~SearchCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) {
        Android::Prepare();
        return true;
    }
    void usage();
    bool SearchObjects(const char* classsname, art::mirror::Object& object);
private:
    uint64_t total_objects;
    int type_flag;
    int obj_each_flags;
    int ref_each_flags;
    bool regex;
    bool instof;
    bool show;
    bool format_hex;
};

#endif // PARSER_COMMAND_ANDROID_CMD_SEARCH_H_
