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

#ifndef PARSER_COMMAND_COMMAND_H_
#define PARSER_COMMAND_COMMAND_H_

#include <string>
#include <unistd.h>
#include <iostream>

class Command {
public:
    Command(const char* c) { command = c; }
    Command(const char* c, const char* s) { command = c; shortcut_cmd = s; }
    Command(const char* c, bool child) { command = c; need_child = child; }
    Command(const char* c, const char* s, bool child) { command = c; shortcut_cmd = s; need_child = child; }
    inline std::string& get() { return command; }
    inline std::string& shortcut() { return shortcut_cmd; }
    inline bool NeedChildMain() { return need_child; }
    virtual ~Command() {}
    virtual int main(int argc, char* const argv[]) = 0;
    virtual void usage() = 0;
    static void Exit(int) { _exit(0); }
    int execute(int argc, char* const argv[]);
private:
    std::string command;
    std::string shortcut_cmd;
    bool need_child;
};

#endif // PARSER_COMMAND_COMMAND_H_
