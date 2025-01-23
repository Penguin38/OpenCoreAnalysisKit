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

#ifndef PARSER_COMMAND_ANDROID_CMD_FDTRACK_H_
#define PARSER_COMMAND_ANDROID_CMD_FDTRACK_H_

#include "command/command.h"
#include "fdtrack/fdtrack.h"
#include <string>
#include <vector>
#include <array>

class FdtrackCommand : public Command {
public:
    FdtrackCommand() : Command("fdtrack") {}
    ~FdtrackCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) { return true; }
    void usage();

    struct NativeFrame {
        uint64_t id;
        uint64_t offset;
        uint64_t pc;
        std::string method;
    };

    struct SortKey {
        uint32_t crc32;
        uint32_t count;
    };

    static void ShowStack(std::vector<NativeFrame>& nfv);
    static void ShowTopStack(std::array<std::vector<NativeFrame>, android::FdTrack::kFdTableSize> fdv, uint32_t num);
private:
    bool dump_top;
    int top;
};

#endif // PARSER_COMMAND_ANDROID_CMD_FDTRACK_H_
