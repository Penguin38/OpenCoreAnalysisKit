/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef PARSER_COMMAND_CMD_INI_H_
#define PARSER_COMMAND_CMD_INI_H_

#include "command/command.h"
#include <unordered_map>
#include <string>

class IniCommand : public Command {
public:
    IniCommand();
    ~IniCommand() {}

    struct Options : Command::Options {
        bool load;
        bool store;
        bool dump_all;
    };

    int main(int argc, char* const argv[]);
    int prepare(int argc, char* const argv[]);
    void usage();
    static uint32_t OffsetValue(void* offset);
    static void SetValue(void* offset, uint32_t value);
    static void SetKeyValue(std::string& key, uint32_t value,
                            std::unordered_map<std::string, void *>& table);
    static void ShowIniTable(const char* section, std::unordered_map<std::string, void *>& table);
private:
    Options options;
    std::unordered_map<std::string, std::unordered_map<std::string, void *>*> android_sections;
    std::unordered_map<std::string, void *> android_offsets;
    std::unordered_map<std::string, void *> android_sizes;
    std::unordered_map<std::string, void *> android_others;
};

#endif // PARSER_COMMAND_CMD_INI_H_
