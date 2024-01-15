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
#include "command/cmd_exec.h"
#include "api/core.h"

int ExecCommand::main(int argc, char* const argv[]) {
    if (CoreApi::IsReady() && argc > 0) {
        CoreApi::ExecFile(argv[0]);
    }
    return 0;
}

void ExecCommand::usage() {
    LOGI("Usage: exec /system/bin/app_process64\n");
}