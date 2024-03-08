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
#include "command/cmd_write.h"
#include "base/utils.h"
#include "api/core.h"

int WriteCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || (argc < 3))
        return 0;

    uint64_t address = Utils::atol(argv[1]) & CoreApi::GetVabitsMask();
    uint64_t value = Utils::atol(argv[2]);
    CoreApi::Write(address, value);
    return 0;
}

void WriteCommand::usage() {
    LOGI("Usage: write|wd address value\n");
}
