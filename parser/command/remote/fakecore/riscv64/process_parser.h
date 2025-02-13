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

#ifndef PARSER_COMMAND_REMOTE_FAKE_CORE_RISCV64_PROCESS_PARSER_H_
#define PARSER_COMMAND_REMOTE_FAKE_CORE_RISCV64_PROCESS_PARSER_H_

#include "command/remote/fakecore/process_parser.h"
#include "common/prstatus.h"
#include <stdint.h>
#include <sys/types.h>

namespace riscv64 {

class ProcessParser : public fakecore::ProcessParser {
public:
    ProcessParser(int pid) : fakecore::ProcessParser(pid) {}
    bool parse();
    bool parseRegister();
    bool parseMaps();

    void * Regs() { return (void *)&regs; }
private:
    struct pt_regs regs;
};

} // namespace riscv64

#endif  // PARSER_COMMAND_REMOTE_FAKE_CORE_RISCV64_PROCESS_PARSER_H_
