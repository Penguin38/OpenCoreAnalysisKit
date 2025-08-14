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

#ifndef PARSER_COMMAND_REMOTE_HOOK_X86_HOOK_IMPL_H_
#define PARSER_COMMAND_REMOTE_HOOK_X86_HOOK_IMPL_H_

#include "common/prstatus.h"
#include "command/remote/hook/lp32/hook.h"

namespace x86 {

class Hook : public lp32::HookImpl {
public:
    Hook(int pid) : lp32::HookImpl(pid) {}
    int GetMachine() { return EM_386; }
    bool InjectLibrary(const char* library);
    uint64_t RegsSize() { return sizeof(pt_regs); }
private:
    pt_regs ori_regs;
};

} // namespace x86

#endif  // PARSER_COMMAND_REMOTE_HOOK_X86_HOOK_IMPL_H_
