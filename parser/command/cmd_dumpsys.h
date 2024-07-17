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

#ifndef PARSER_COMMAND_CMD_DUMPSYS_H_
#define PARSER_COMMAND_CMD_DUMPSYS_H_

#include "command/command.h"

class DumpsysCommand : public Command {
public:
    DumpsysCommand() : Command("dumpsys") {}
    ~DumpsysCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) {
        Android::Prepare();
        return true;
    }
    void usage();
};

#endif // PARSER_COMMAND_CMD_DUMPSYS_H_
