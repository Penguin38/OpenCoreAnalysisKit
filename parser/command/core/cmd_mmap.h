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

#ifndef PARSER_COMMAND_CORE_CMD_MMAP_H_
#define PARSER_COMMAND_CORE_CMD_MMAP_H_

#include "command/command.h"

class MmapCommand : public Command {
public:
    static constexpr int REMOVE_MMAP = 1 << 0;
    static constexpr int REMOVE_OVERLAY = 1 << 1;

    MmapCommand() : Command("mmap") {}
    ~MmapCommand() {}
    int main(int argc, char* const argv[]);
    void usage();
};

#endif // PARSER_COMMAND_CORE_CMD_MMAP_H_
