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

#include "api/core.h"
#include "api/thread.h"
#include "arm64/thread_info.h"
#include "arm/thread_info.h"
#include "base/utils.h"
#include "common/exception.h"
#include "common/bit.h"
#include <string.h>
#include <iomanip>
#include <iostream>

int main(int argc, const char* argv[]) {
    std::cout << CoreApi::Load(argv[1]) << std::endl;
    std::cout << CoreApi::GetMachineName() << std::endl;

    CoreApi::Write(0x12c00018, 0xdeaddeaddeaddeadUL);
    CoreApi::Write(0x6f718f2000, 0xdeaddeaddeaddeadUL);

    uint8_t value[0x100];
    memset(value, 0x0, sizeof(value));
    uint64_t beginaddr = 0x12c00018;
    CoreApi::Read(beginaddr, sizeof(value), value);
    int count = sizeof(value) / sizeof(uint64_t);
    for (int index = 0; index < count; index += 2) {
        std::cout << std::hex << (beginaddr + index * 0x8) << ": "
                  << std::setw(16) << std::setfill('0') << reinterpret_cast<uint64_t *>(value)[index] << "  "
                  << std::setw(16) << std::setfill('0') << reinterpret_cast<uint64_t *>(value)[index + 1] << "  "
                  << Utils::ConvertAscii(reinterpret_cast<uint64_t *>(value)[index], 8)
                  << Utils::ConvertAscii(reinterpret_cast<uint64_t *>(value)[index + 1], 8) << std::endl;
    }

    std::cout << std::endl;
    memset(value, 0x0, sizeof(value));
    CoreApi::Read(beginaddr, sizeof(value), value, OPT_READ_MMAP);
    for (int index = 0; index < count; index += 2) {
        std::cout << std::hex << (beginaddr + index * 0x8) << ": "
                  << std::setw(16) << std::setfill('0') << reinterpret_cast<uint64_t *>(value)[index] << "  "
                  << std::setw(16) << std::setfill('0') << reinterpret_cast<uint64_t *>(value)[index + 1] << "  "
                  << Utils::ConvertAscii(reinterpret_cast<uint64_t *>(value)[index], 8)
                  << Utils::ConvertAscii(reinterpret_cast<uint64_t *>(value)[index + 1], 8) << std::endl;
    }

    std::cout << std::hex << *(uint64_t *)CoreApi::GetReal(0x12c00018) << std::endl;
    std::cout << std::hex << CoreApi::GetVirtual(CoreApi::GetReal(0x12c00018)) << std::endl;
    try {
        std::cout << std::hex << *(uint64_t *)CoreApi::GetReal(0x0000007e19f14100UL) << std::endl;
    } catch (InvalidAddressException e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << std::hex << RoundUp(0x1994, 0x1000) << std::endl;
    std::cout << std::hex << RoundDown(0x1994, 0x1000) << std::endl;
    std::cout << std::hex << CoreApi::FindAuxv(AT_EXECFN) << std::endl;

    ThreadApi* thread = CoreApi::FindThread(1745);
    if (thread && !strcmp(CoreApi::GetMachineName(), "arm64")) {
        arm64::ThreadInfo* info = reinterpret_cast<arm64::ThreadInfo *>(thread);
        std::cout << std::hex << info->reg.pc << std::endl;
    }

    ThreadApi* thread1 = CoreApi::FindThread(7836);
    std::cout << "thread1 " << thread1 << std::endl;
    if (thread1 && !strcmp(CoreApi::GetMachineName(), "arm")) {
        arm::ThreadInfo* info = reinterpret_cast<arm::ThreadInfo *>(thread1);
        std::cout << std::hex << info->reg.pc << std::endl;
    }

    // std::cout << std::hex << CoreApi::GetDebug() << std::endl;
#ifdef __ANDROID__
    CoreApi::ExecFile("/system");
    CoreApi::SysRoot("/system:/vendor");
#endif
    // CoreApi::DumpFile();
    // CoreApi::DumpLinkMap();
    CoreApi::UnLoad();
    return 0;
}
