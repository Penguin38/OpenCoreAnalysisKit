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

#ifndef PARSER_COMMAND_REMOTE_HOOK_ARM64_HOOK_IMPL_H_
#define PARSER_COMMAND_REMOTE_HOOK_ARM64_HOOK_IMPL_H_

#include "common/prstatus.h"
#include "command/remote/hook/lp64/hook.h"

namespace arm64 {

class Hook : public lp64::HookImpl {
public:
    Hook(int pid) : lp64::HookImpl(pid) {}
    uint64_t RegsSize() { return sizeof(pt_regs); }
    bool InjectLibrary(const char* library);
    bool CallMethod(const char* method, int argc, char* const argv[]);
    bool MethodContext(const char* method, pt_regs* call_regs,
                       int argc, char* const argv[]);
private:
    pt_regs ori_regs;
};

} // namespace arm64

#endif  // PARSER_COMMAND_REMOTE_HOOK_ARM64_HOOK_IMPL_H_
