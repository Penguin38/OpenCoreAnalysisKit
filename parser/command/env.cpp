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

#include "logger/log.h"
#include "command/env.h"
#include "api/core.h"
#include "android.h"
#include "llvm.h"

std::unique_ptr<Env> Env::INSTANCE = nullptr;

bool Env::setCurrentPid(int p) {
    ThreadApi *api = CoreApi::FindThread(p);
    if (!api) {
        LOGE("Not find tid: " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, p);
        return false;
    }
    pid = api->pid();
    return true;
}

void Env::init() {
    auto callback = [&](ThreadApi *api) -> bool {
        pid = api->pid();
        if (CoreApi::IsRemote())
            remote_pid = pid;
        return true;
    };
    CoreApi::ForeachThread(callback);

    CoreApi::Dump();
    Env::Dump();

    LLVM::Init();

#if defined(__AOSP_PARSER__)
    Android::Init();
    Android::Dump();
#endif
}

void Env::Init() {
    INSTANCE = std::make_unique<Env>();
    INSTANCE->init();
}

void Env::Dump() {
    LOGI("  * Thread: " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, CurrentPid());
}
