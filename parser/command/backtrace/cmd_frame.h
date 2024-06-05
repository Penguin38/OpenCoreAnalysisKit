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

#ifndef PARSER_COMMAND_BACKTRACE_CMD_FRAME_H_
#define PARSER_COMMAND_BACKTRACE_CMD_FRAME_H_

#include "command/command.h"
#include "android.h"

class FrameCommand : public Command {
public:
    FrameCommand() : Command("frame", "f") {}
    ~FrameCommand() {}
    int main(int argc, char* const argv[]);
    bool prepare(int argc, char* const argv[]) {
#if defined(__AOSP_PARSER__)
        Android::Prepare();
        Android::OatPrepare();
#endif
        return true;
    }
    void usage();
    void ShowJavaFrameInfo(int number);
private:
    bool java;
};

#endif // PARSER_COMMAND_BACKTRACE_CMD_FRAME_H_
