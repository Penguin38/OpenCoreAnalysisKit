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

#include "command/fake/core/tombstone.h"
#include "command/fake/core/arm64/tombstone_parser.h"
#include <stdio.h>

namespace android {

Tombstone::Tombstone(const char* path) {
    FILE *fp = fopen(path, "r");
    char line[1024];
    char machine[16];
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (sscanf(line, "ABI: '%[^']'", machine)) {
                abi = machine;
                break;
            }
        }
        fclose(fp);
    }
    mParser = MakeParser(abi.c_str(), path);
}

bool Tombstone::Parse() {
    return mParser ? mParser->parse() : false;
}

std::unique_ptr<TombstoneParser> Tombstone::MakeParser(const char* abi, const char* path) {
    std::unique_ptr<TombstoneParser> impl;
    std::string type = abi;
    if (type == "arm64" || type == "ARM64") {
        impl = std::make_unique<arm64::TombstoneParser>(path);
    }
    return std::move(impl);
}

} // namespace android
