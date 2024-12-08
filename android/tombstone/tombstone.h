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

#ifndef ANDROID_TOMBSTONE_TOMBSTONE_H_
#define ANDROID_TOMBSTONE_TOMBSTONE_H_

#include "tombstone/parser.h"
#include <string>
#include <memory>

namespace android {

class Tombstone {
public:
    Tombstone(const char* path);
    std::string& ABI() { return abi; }
    static std::unique_ptr<TombstoneParser> MakeParser(const char* abi, const char* path);
private:
    std::unique_ptr<TombstoneParser> mParser;
    std::string abi;
};

} // namespace android

#endif  // ANDROID_TOMBSTONE_TOMBSTONE_H_
