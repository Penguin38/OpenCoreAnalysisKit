/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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
#include "api/core.h"
#include "command/fake/exec/fake_executable.h"

int FakeExecutable::OptionExec(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || (argc < 2)) {
        return 0;
    }

    LinkMap* executable = CoreApi::FindLinkMap(argv[1]);
    RebuildExecDynamic(executable);
    return 0;
}

bool FakeExecutable::RebuildExecDynamic(LinkMap* executable) {
    LOGI("Rebuild FAKE PHDR\n");
    if (CoreApi::Bits() == 64)
        return FakeExecutable::RebuildExecDynamic64(executable);
    else
        return FakeExecutable::RebuildExecDynamic32(executable);
}

void FakeExecutable::Usage() {
    LOGI("Usage: fake exec <EXEC_PATH>\n");
    LOGI("core-parser> fake exec /system/bin/app_process64\n");
}
