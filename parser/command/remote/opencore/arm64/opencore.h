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

#ifndef PARSER_COMMAND_REMOTE_OPENCORE_ARM64_OPENCORE_IMPL_H_
#define PARSER_COMMAND_REMOTE_OPENCORE_ARM64_OPENCORE_IMPL_H_

#include "common/elf.h"
#include "common/prstatus.h"
#include "command/remote/opencore/lp64/opencore.h"

namespace arm64 {

class Opencore : public lp64::OpencoreImpl {
public:
    Opencore() : lp64::OpencoreImpl(),
                 prnum(0), prstatus(nullptr) {}
    ~Opencore();
    void CreateCorePrStatus(int pid);
    void WriteCorePrStatus(FILE* fp);
    bool IsSpecialFilterSegment(Opencore::VirtualMemoryArea& vma);
    void WriteCorePAC(int tid, FILE* fp);
    void WriteCoreMTE(int tid, FILE* fp);
    int getMachine() { return EM_AARCH64; }
private:
    int prnum;
    Elf64_prstatus *prstatus;
};

} // namespace arm64

#endif // PARSER_COMMAND_REMOTE_OPENCORE_ARM64_OPENCORE_IMPL_H_
