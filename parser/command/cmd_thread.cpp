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
#include "command/cmd_thread.h"
#include "base/utils.h"
#include "api/core.h"

int ThreadCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    if (argc) {
        int current_pid = atoi(argv[0]);
        Env::SetCurrentPid(current_pid);
    }
    LOGI("Current thread is %d\n", Env::CurrentPid());
    return 0;
}

void ThreadCommand::usage() {
    LOGI("Usage: thread [tid]");
}
