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

#ifndef CORE_COMMON_SYMENT_H_
#define CORE_COMMON_SYMENT_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>

class SymbolEntry {
public:
    SymbolEntry(uint64_t off, uint64_t ty, const char* name) {
        offset = off;
        type = ty;
        if (name) symbol = name;
    }

    uint64_t offset;
    uint64_t type;
    std::string symbol;

    static bool Compare(SymbolEntry& a, SymbolEntry& b) { return a.offset < b.offset; }
};

#endif // CORE_COMMON_SYMENT_H_
