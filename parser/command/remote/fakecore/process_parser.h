/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#ifndef PARSER_COMMAND_REMOTE_FAKE_CORE_PROCESS_PARSER_H_
#define PARSER_COMMAND_REMOTE_FAKE_CORE_PROCESS_PARSER_H_

#include "command/remote/opencore/opencore.h"
#include <stdio.h>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace fakecore {

class ProcessParser {
public:
    ProcessParser(int pid) {
        mProcessTid = pid;
    }
    ~ProcessParser() {}
    virtual bool parse() { return false; }
    virtual void* Regs() { return nullptr; }
    virtual uint64_t TaggedAddr() { return 0; }
    virtual uint64_t PacEnabledKeys() { return 0; }

    int Tid() { return mProcessTid; }
    std::string Executable() { return mExecutable; }
    std::set<std::string>& Libs() { return mLibs; }
    std::map<uint64_t, uint64_t>& Memorys() { return mMemorys; }
    std::vector<Opencore::VirtualMemoryArea>& Maps() { return mMaps; }
protected:
    int mProcessTid;
    std::string mExecutable;
    std::set<std::string> mLibs;
    std::map<uint64_t, uint64_t> mMemorys;
    std::vector<Opencore::VirtualMemoryArea> mMaps;
};

} // namespace fakecore

#endif  // PARSER_COMMAND_REMOTE_FAKE_CORE_PROCESS_PARSER_H_
