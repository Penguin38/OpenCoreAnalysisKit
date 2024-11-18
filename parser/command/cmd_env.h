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

#ifndef PARSER_COMMAND_CMD_ENV_H_
#define PARSER_COMMAND_CMD_ENV_H_

#include "command/command.h"

class EnvCommand : public Command {
public:
    EnvCommand() : Command("env") {}
    ~EnvCommand() {}
    int main(int argc, char* const argv[]);
    int dumpEnv();
    void usage();
    static int onConfigChanged(int argc, char* const argv[]);
    static int onLoggerChanged(int argc, char* const argv[]);
    static int showArtEnv(int argc, char* const argv[]);
    static int showCoreEnv(int argc, char* const argv[]);
    static int showLoadEnv(bool quick);
    static int showNoteEnv();
    static int clocLoadCRC32(int num);
};

#endif // PARSER_COMMAND_CMD_ENV_H_
