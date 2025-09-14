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

#ifndef PARSER_COMMAND_ANDROID_CMD_CLASS_H_
#define PARSER_COMMAND_ANDROID_CMD_CLASS_H_

#include "command/command.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include <string>

class ClassCommand : public Command {
public:
    static constexpr int SHOW_METHOD = 1 << 0;
    static constexpr int SHOW_IMPL = 1 << 1;
    static constexpr int SHOW_STATIC = 1 << 2;
    static constexpr int SHOW_FIELD = 1 << 3;
    static constexpr int SHOW_ALL = SHOW_METHOD | SHOW_IMPL |
                                    SHOW_STATIC | SHOW_FIELD;

    ClassCommand() : Command("class") {}
    ~ClassCommand() {}

    struct Options : Command::Options {
        uint64_t total_classes = 0;
        bool dump_all          = false;
        bool format_hex        = false;
        int show_flag          = 0;
        int obj_each_flags     = 0;
    };

    int main(int argc, char* const argv[]);
    int prepare(int argc, char* const argv[]);
    void usage();

    bool PrintClass(art::mirror::Object& object, const char* classname);
    static void PrintPrettyClassContent(art::mirror::Class& clazz, ClassCommand::Options& options);
private:
    Options options;
};

#endif // PARSER_COMMAND_ANDROID_CMD_CLASS_H_
