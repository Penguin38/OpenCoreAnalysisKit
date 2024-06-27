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

#include "logger/log.h"
#include "command/env.h"
#include "command/cmd_core.h"
#include "android.h"
#include "llvm.h"
#include "api/core.h"

int CoreCommand::Load(const char* path) {
    bool ret = CoreApi::Load(path);
    if (ret) {
        CoreApi::Init();
        CoreApi::Dump();

        Env::Clean();
        Env::Init();
        Env::Dump();

        // symbols init for later
        LLVM::Init();

#if defined(__AOSP_PARSER__)
        Android::Clean(); // clean prev core env
        Android::Init();
        Android::Dump();
#endif
    }
    return ret;
}

int CoreCommand::main(int argc, char* const argv[]) {
    return Load(argv[1]);
}

void CoreCommand::usage() {
    LOGI("Usage: core /tmp/default.core\n");
}
