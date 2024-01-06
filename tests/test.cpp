#include "api/core.h"
#include "common/exception.h"
#include "common/bit.h"
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
    CoreApi::UnLoad();
    return 0;
}
