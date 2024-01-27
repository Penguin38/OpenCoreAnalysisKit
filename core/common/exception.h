/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
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

#ifndef CORE_COMMON_EXCEPTION_H_
#define CORE_COMMON_EXCEPTION_H_

#include "logger/log.h"
#include "backtrace/callstack.h"
#include <stdint.h>
#include <sys/types.h>
#include <sstream>
#include <exception>
#include <string>

class InvalidAddressException : public std::exception {
private:
    uint64_t addr;
    std::string error;
public:
    InvalidAddressException(uint64_t p) : addr(p) {
        if (Logger::IsDebug()) {
            CallStack stack;
            stack.update(2);
            stack.dump(1);
        }

        std::ostringstream ss;
        ss << "Invalid address 0x" << std::hex << addr;
        error.append(ss.str());
    }

    const char * what() const throw() {
        return error.c_str();
    }
};

#endif  // CORE_COMMON_EXCEPTION_H_
