#include "api/core.h"
#include <iostream>

int main(int argc, const char* argv[]) {
    std::cout << CoreApi::Load(argv[1]) << std::endl;
    std::cout << CoreApi::GetMachine() << std::endl;
    CoreApi::UnLoad();
    return 0;
}
