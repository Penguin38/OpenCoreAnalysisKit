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

#ifndef PARSER_COMMAND_FAKE_CORE_TOMBSTONE_H_
#define PARSER_COMMAND_FAKE_CORE_TOMBSTONE_H_

#include "command/fake/core/tombstone_parser.h"
#include "command/fake/core/fake_core.h"
#include <string>
#include <memory>

namespace android {

class Tombstone : public FakeCore::Stream {
public:
	// FakeCore::Stream API
	std::string ABI() { return abi; }
    bool Parse();
    int Tid() { return mParser->Tid(); }
    std::set<std::string>& Libs() { return mParser->Libs(); }
    std::map<uint64_t, uint64_t>& Memorys() { return mParser->Memorys(); }
    std::vector<Opencore::VirtualMemoryArea>& Maps() { return mParser->Maps(); }
    void* Regs() { return mParser->Regs(); }
    uint64_t TaggedAddr() { return mParser->TaggedAddr(); }
    uint64_t PacEnabledKeys() { return mParser->PacEnabledKeys(); }

    Tombstone(const char* path);
    static std::unique_ptr<TombstoneParser> MakeParser(const char* abi, const char* path);
private:
    std::string abi;
    std::unique_ptr<TombstoneParser> mParser;
};

} // namespace android

#endif  // PARSER_COMMAND_FAKE_CORE_TOMBSTONE_H_
