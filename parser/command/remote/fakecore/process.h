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

#ifndef PARSER_COMMAND_REMOTE_FAKE_CORE_PROCESS_H_
#define PARSER_COMMAND_REMOTE_FAKE_CORE_PROCESS_H_

#include "command/remote/fakecore/process_parser.h"
#include "command/fake/core/fake_core.h"
#include <string>
#include <memory>

namespace fakecore {

class Process : public FakeCore::Stream {
public:
	// FakeCore::Stream API
	std::string ABI() { return abi; }
    bool Parse();
    int Tid() { return mParser->Tid(); }
    std::string Executable() { return mParser->Executable(); }
    void SetExecutable(const char* path) {}
    std::set<std::string>& Libs() { return mParser->Libs(); }
    std::map<uint64_t, uint64_t>& Memorys() { return mParser->Memorys(); }
    std::vector<Opencore::VirtualMemoryArea>& Maps() { return mParser->Maps(); }
    void* Regs() { return mParser->Regs(); }
    uint64_t TaggedAddr() { return mParser->TaggedAddr(); }
    uint64_t PacEnabledKeys() { return mParser->PacEnabledKeys(); }

    Process(int pid);
    static std::unique_ptr<ProcessParser> MakeParser(const char* abi, int pid);
private:
    std::string abi;
    std::unique_ptr<ProcessParser> mParser;
};

} // namespace fakecore

#endif  // PARSER_COMMAND_REMOTE_FAKE_CORE_PROCESS_H_
