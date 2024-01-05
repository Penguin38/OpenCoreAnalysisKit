#include "api/core.h"
#include "common/exception.h"
#include <iostream>

int main(int argc, const char* argv[]) {
    std::cout << CoreApi::Load(argv[1]) << std::endl;
    std::cout << CoreApi::GetMachine() << std::endl;
    std::cout << std::hex << *(uint64_t *)CoreApi::GetReal(0x12c00000) << std::endl;
    std::cout << std::hex << CoreApi::GetVirtual(CoreApi::GetReal(0x12c00000)) << std::endl;
    try {
        std::cout << std::hex << *(uint64_t *)CoreApi::GetReal(0x12000000) << std::endl;
    } catch (InvalidAddressException e) {
        std::cout << e.what() << std::endl;
    }
    CoreApi::UnLoad();
    return 0;
}
