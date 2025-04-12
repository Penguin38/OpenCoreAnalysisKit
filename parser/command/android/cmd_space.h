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

#ifndef PARSER_COMMAND_ANDROID_CMD_SPACE_H_
#define PARSER_COMMAND_ANDROID_CMD_SPACE_H_

#include "command/command.h"

class SpaceCommand : public Command {
public:
    SpaceCommand() : Command("space") {}
    ~SpaceCommand() {}

    struct Options : Command::Options {
        int flag;
        bool check;
    };

    int main(int argc, char* const argv[]);
    int prepare(int argc, char* const argv[]);
    void usage();
private:
    Options options;
};

#endif // PARSER_COMMAND_ANDROID_CMD_SPACE_H_
