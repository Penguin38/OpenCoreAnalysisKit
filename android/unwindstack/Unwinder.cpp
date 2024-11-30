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

#include "unwindstack/Unwinder.h"
#include "api/core.h"
#include "cxx/string.h"
#include <cxxabi.h>

struct FrameData_OffsetTable __FrameData_offset__;
struct FrameData_SizeTable __FrameData_size__;

namespace android {

void UnwindStack::Init() {
    android::UnwindStack::FrameData::Init();
}

void UnwindStack::FrameData::Init() {
    if (CoreApi::Bits() == 64) {
        __FrameData_offset__ = {
            .pc = 16,
            .function_name = 32,
            .function_offset = 48,
        };

        __FrameData_size__ = {
            .THIS = 72,
        };
    } else {
        __FrameData_offset__ = {
            .pc = 16,
            .function_name = 32,
            .function_offset = 40,
        };

        __FrameData_size__ = {
            .THIS = 56,
        };
    }
}

std::string UnwindStack::FrameData::GetMethod() {
    int status;
    std::string method;
    cxx::string name = function_name();
    std::string symbol = name.c_str();
    char* demangled_name = abi::__cxa_demangle(symbol.c_str(), nullptr, nullptr, &status);
    if (status == 0) {
        method = demangled_name;
        std::free(demangled_name);
    } else {
        method = symbol;
    }
    return method;
}

} // namespace android
