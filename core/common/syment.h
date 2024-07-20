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
#include <functional>

class SymbolEntry {
public:
    SymbolEntry(uint64_t off, uint64_t ty, uint64_t cs, const char* name) {
        offset = off;
        type = ty;
        size = cs;
        if (name) symbol = name;
    }

    uint64_t offset;
    uint64_t type;
    uint64_t size;
    std::string symbol;

    bool operator==(const SymbolEntry& entry) const {
        return offset == entry.offset
                && type == entry.type
                && size == entry.size;
    }

    struct Hash {
        std::size_t operator()(const SymbolEntry& entry) const {
            std::hash<uint64_t> offset_hasher;
            std::hash<uint64_t> type_hasher;
            std::hash<uint64_t> size_hasher;
            return offset_hasher(entry.offset) ^ type_hasher(entry.type) ^ size_hasher(entry.size);
        }
    };
};

#endif // CORE_COMMON_SYMENT_H_
