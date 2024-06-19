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

#ifndef PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_
#define PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_

#include <string>

class Opencore {
public:
    static const int FLAG_CORE = 1 << 0;
    static const int FLAG_PROCESS_COMM = 1 << 1;
    static const int FLAG_PID = 1 << 2;
    static const int FLAG_THREAD_COMM = 1 << 3;
    static const int FLAG_TID = 1 << 4;
    static const int FLAG_TIMESTAMP = 1 << 5;
    static const int FLAG_ALL = FLAG_CORE | FLAG_PROCESS_COMM | FLAG_PID
                              | FLAG_THREAD_COMM | FLAG_TID | FLAG_TIMESTAMP;

    static const int INVALID_TID = 0;

    static const int FILTER_NONE = 0x0;
    static const int FILTER_SPECIAL_VMA = 1 << 0;
    static const int FILTER_FILE_VMA = 1 << 1;
    static const int FILTER_SHARED_VMA = 1 << 2;
    static const int FILTER_SANITIZER_SHADOW_VMA = 1 << 3;
    static const int FILTER_NON_READ_VMA = 1 << 4;

    static int Dump(int argc, char* const argv[]);
    static void Usage();

    Opencore() {
        flag = FLAG_CORE
             | FLAG_PID
             | FLAG_PROCESS_COMM
             | FLAG_TIMESTAMP;
        pid = INVALID_TID;
        filter = FILTER_NONE;
        extra_note_filesz = 0;
    }

    virtual bool DoCoredump(const char* filename) = 0;
private:
    std::string dir;
    int flag;
    int pid;
    int filter;
    int extra_note_filesz;
};

#endif // PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_
