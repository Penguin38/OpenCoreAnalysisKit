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

#ifndef PARSER_COMMAND_REMOTE_OPENCORE_X86_OPENCORE_IMPL_H_
#define PARSER_COMMAND_REMOTE_OPENCORE_X86_OPENCORE_IMPL_H_

#include "common/elf.h"
#include "common/prstatus.h"
#include "command/remote/opencore/lp32/opencore.h"

namespace x86 {

class Opencore : public lp32::OpencoreImpl {
public:
    Opencore() : lp32::OpencoreImpl(),
                 prnum(0), prstatus(nullptr) {}
    ~Opencore();
    void CreateCorePrStatus(int pid);
    void WriteCorePrStatus(FILE* fp);
    bool IsSpecialFilterSegment(Opencore::VirtualMemoryArea& vma, int idx);
    int getMachine() { return EM_386; }
private:
    int prnum;
    Elf32_prstatus *prstatus;
};

} // namespace x86

#endif // PARSER_COMMAND_REMOTE_OPENCORE_X86_OPENCORE_IMPL_H_
