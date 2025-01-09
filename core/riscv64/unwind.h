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

#ifndef CORE_RISCV64_UNWIND_H_
#define CORE_RISCV64_UNWIND_H_

#include "api/unwind.h"
#include "riscv64/thread_info.h"

namespace riscv64 {

class UnwindStack : public api::UnwindStack {
public:
    UnwindStack(ThreadApi* thread) : api::UnwindStack(thread) {}
    void WalkStack();
    uint64_t GetUContext();
    void DumpContextRegister(const char* prefix);
};

} // namespace riscv64

#endif // CORE_RISCV64_UNWIND_H_
