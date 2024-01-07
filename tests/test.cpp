#include "api/core.h"
#include "api/thread.h"
#include "arm64/thread_info.h"
#include "arm/thread_info.h"
#include "common/exception.h"
#include "common/bit.h"
#include <string.h>
#include <iostream>

int main(int argc, const char* argv[]) {
    std::cout << CoreApi::Load(argv[1]) << std::endl;
    std::cout << CoreApi::GetMachine() << std::endl;
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
    if (thread && !strcmp(CoreApi::GetMachine(), "arm64")) {
        arm64::ThreadInfo* info = reinterpret_cast<arm64::ThreadInfo *>(thread);
        std::cout << std::hex << info->reg.pc << std::endl;
    }

    ThreadApi* thread1 = CoreApi::FindThread(7836);
    std::cout << "thread1 " << thread1 << std::endl;
    if (thread1 && !strcmp(CoreApi::GetMachine(), "arm")) {
        arm::ThreadInfo* info = reinterpret_cast<arm::ThreadInfo *>(thread1);
        std::cout << std::hex << info->reg.pc << std::endl;
    }

    CoreApi::DumpFile();
    CoreApi::UnLoad();
    return 0;
}
