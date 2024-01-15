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
#include "command/cmd_file.h"
#include "base/utils.h"
#include "api/core.h"
#include <stdio.h>

int FileCommand::main(int argc, char* const argv[]) {
    if (CoreApi::IsReady())  {
        if (!argc) {
            CoreApi::DumpFile();
        } else {
            uint64_t address = Utils::atol(argv[0]);
            auto callback = [address](File* file) -> bool {
                if (address >= file->begin() && address < file->end()) {
                    LOGI("[%lx, %lx) %lx %s\n", file->begin(), file->end(), file->offset(), file->name().c_str());
                    return true;
                }
                return false;
            };
            CoreApi::ForeachFile(callback);
        }
    }
    return 0;
}

void FileCommand::usage() {
    LOGI("Usage: file [virtaul address]\n");
}