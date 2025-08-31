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
#include "command/remote/opencore/opencore.h"
#include "command/remote/fakecore/arm64/process_parser.h"
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/uio.h>

namespace arm64 {

bool ProcessParser::parse() {
    tagged_addr_ctrl = -1;
    pac_enabled_keys = -1;
    memset(&regs, 0x0, sizeof(regs));

    bool ret = parsePauth() &&
               parseRegister() &&
               parseMaps();

    LOGI("Tid: %d\n", mProcessTid);
    if (tagged_addr_ctrl) LOGI("tagged_addr_ctrl %" PRIx64 "\n", tagged_addr_ctrl);
    if (pac_enabled_keys) LOGI("pac_enabled_keys %" PRIx64 "\n", pac_enabled_keys);
    return ret;
}

bool ProcessParser::parsePauth() {
    std::unique_ptr<Opencore> opencore = std::make_unique<Opencore>();
    opencore->StopTheWorld(mProcessTid);

    struct iovec pac_enabled_keys_iov = {
        &pac_enabled_keys,
        sizeof(pac_enabled_keys),
    };
    if (ptrace(PTRACE_GETREGSET, mProcessTid, NT_ARM_PAC_ENABLED_KEYS,
                reinterpret_cast<void*>(&pac_enabled_keys_iov)) == -1) {
        pac_enabled_keys = -1;
    }

    struct iovec tagged_addr_ctrl_iov = {
        &tagged_addr_ctrl,
        sizeof(tagged_addr_ctrl),
    };
    if (ptrace(PTRACE_GETREGSET, mProcessTid, NT_ARM_TAGGED_ADDR_CTRL,
                reinterpret_cast<void*>(&tagged_addr_ctrl_iov)) == -1) {
        tagged_addr_ctrl = -1;
    }
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
