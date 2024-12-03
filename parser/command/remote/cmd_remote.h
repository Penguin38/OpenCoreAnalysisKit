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

#ifndef PARSER_COMMAND_REMOTE_CMD_REMOTE_H_
#define PARSER_COMMAND_REMOTE_CMD_REMOTE_H_

#include "command/command.h"
#include <string>

class RemoteCommand : public Command {
public:
    RemoteCommand() : Command("remote") {}
    ~RemoteCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) { return true; }
    void usage();
    static int OptionRead(int argc, char* const argv[]);
    static int OptionWrite(int argc, char* const argv[]);
    static int OptionPause(int argc, char* const argv[]);
    static int OptionMaps(int argc, char* const argv[]);
    static void Write(int pid, uint64_t vaddr, uint64_t value) {
        Write(pid, vaddr, &value, 8);
    }
    static void Write(int pid, uint64_t vaddr, void *buf, uint64_t size);
    static bool Read(int pid, uint64_t vaddr, uint64_t size, uint8_t* buf);
};

#endif // PARSER_COMMAND_REMOTE_CMD_REMOTE_H_
