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

#ifndef PARSER_COMMAND_LLVM_CMD_CXX_H_
#define PARSER_COMMAND_LLVM_CMD_CXX_H_

#include "command/command.h"

class CxxCommand : public Command {
public:
    CxxCommand() : Command("cxx") {}
    ~CxxCommand() {}
    int main(int argc, char* const argv[]);
    int prepare(int argc, char* const argv[]);
    void usage();

    static int DumpCxxString(int argc, char* const argv[]);
    static int DumpCxxVector(int argc, char* const argv[]);
    static int DumpCxxMap(int argc, char* const argv[]);
    static int DumpCxxSet(int argc, char* const argv[]);
    static int DumpCxxUnOrderedMap(int argc, char* const argv[]);
    static int DumpCxxUnOrderedSet(int argc, char* const argv[]);
    static int DumpCxxList(int argc, char* const argv[]);
    static int DumpCxxDeque(int argc, char* const argv[]);
};

#endif // PARSER_COMMAND_LLVM_CMD_CXX_H_
