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

#ifndef PARSER_COMMAND_CMD_CLASS_H_
#define PARSER_COMMAND_CMD_CLASS_H_

#include "command/command.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "android.h"
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
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) {
        Android::Prepare();
        return true;
    }
    void usage();
    bool PrintClass(art::mirror::Object& object, const char* classname);
    void PrintPrettyClassContent(art::mirror::Class& clazz);
    void PrintField(const char* format, art::mirror::Class& clazz, art::ArtField& field);
private:
    uint64_t total_classes;
    bool dump_all;
    int show_flag;
};

#endif // PARSER_COMMAND_CMD_CLASS_H_
