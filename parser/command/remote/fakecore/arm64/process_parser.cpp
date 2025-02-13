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
#include "command/remote/fakecore/arm64/process_parser.h"
#include <stdio.h>
#include <string.h>

namespace arm64 {

bool ProcessParser::parse() {
    tagged_addr_ctrl = -1;
    pac_enabled_keys = -1;
    memset(&regs, 0x0, sizeof(regs));

    bool ret = parsePauth() &&
               parseRegister() &&
               parseMaps();

    LOGI("Tid: %d\n", mProcessTid);
    return ret;
}

bool ProcessParser::parsePauth() {
    return true;
}

bool ProcessParser::parseRegister() {
    return true;
}

bool ProcessParser::parseMaps() {
    Opencore::ParseMaps(mProcessTid, mMaps);
    return true;
}

} // namespace arm64
