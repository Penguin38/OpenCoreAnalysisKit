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


#ifndef PARSER_COMMAND_CMD_LOGCAT_H_
#define PARSER_COMMAND_CMD_LOGCAT_H_

#include "command/command.h"

class LogcatCommand : public Command {
public:
    static constexpr int DUMP_MAIN = 1 << 0;
    static constexpr int DUMP_RADIO = 1 << 1;
    static constexpr int DUMP_EVENTS = 1 << 2;
    static constexpr int DUMP_SYSTEM = 1 << 3;
    static constexpr int DUMP_CRASH = 1 << 4;
    static constexpr int DUMP_KERNEL = 1 << 5;

    LogcatCommand() : Command("logcat") {}
    ~LogcatCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) { return true; }
    void usage();
private:
    int dump_flag = 0;
    int filter = 0;
    int id;
};

#endif // PARSER_COMMAND_CMD_LOGCAT_H_
