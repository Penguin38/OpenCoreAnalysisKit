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

#include "command/remote/fakecore/process.h"
#include "command/remote/fakecore/arm64/process_parser.h"
#include "command/remote/fakecore/arm/process_parser.h"
#include "command/remote/fakecore/x86_64/process_parser.h"
#include "command/remote/fakecore/x86/process_parser.h"
#include "command/remote/fakecore/riscv64/process_parser.h"
#include "command/remote/opencore/opencore.h"
#include <stdio.h>

namespace fakecore {

Process::Process(int pid) {
    abi = Opencore::DecodeMachine(pid);
    mParser = MakeParser(abi.c_str(), pid);
}

bool Process::Parse() {
    return mParser ? mParser->parse() : false;
}

std::unique_ptr<ProcessParser> Process::MakeParser(const char* abi, int pid) {
    std::unique_ptr<ProcessParser> impl;
    std::string type = abi;
#if defined(__LP64__)
    if (type == "arm64" || type == "ARM64") {
        impl = std::make_unique<arm64::ProcessParser>(pid);
    } else if (type == "x86_64" || type == "X86_64") {
        impl = std::make_unique<x86_64::ProcessParser>(pid);
    } else if (type == "riscv64" || type == "RISCV64") {
        impl = std::make_unique<riscv64::ProcessParser>(pid);
    } else if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::ProcessParser>(pid);
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::ProcessParser>(pid);
    }
#else
    if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::ProcessParser>(pid);
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::ProcessParser>(pid);
    }
#endif
    return std::move(impl);
}

} // namespace fakecore
