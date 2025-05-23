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

class SearchCommand : public Command {
public:
    static constexpr int SEARCH_OBJECT = 1 << 0;
    static constexpr int SEARCH_CLASS = 1 << 1;

    SearchCommand() : Command("search") {}
    ~SearchCommand() {}

    struct Options : Command::Options {
        uint64_t total_objects  = 0;
        int type_flag           = 0;
        int obj_each_flags      = 0;
        int ref_each_flags      = 0;
        bool regex              = false;
        bool instof             = false;
        bool show               = false;
        bool format_hex         = false;
        bool reference          = false;
        int deep                = 0;
    };

    int main(int argc, char* const argv[]);
    int prepare(int argc, char* const argv[]);
    void usage();
    bool SearchObjects(const char* classsname, art::mirror::Object& object);
private:
    Options options;
};

#endif // PARSER_COMMAND_ANDROID_CMD_SEARCH_H_
