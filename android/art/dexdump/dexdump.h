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

#ifndef ANDROID_ART_DEXDUMP_DEXDUMP_H_
#define ANDROID_ART_DEXDUMP_DEXDUMP_H_

#include "dex/dex_file.h"

namespace art {

class Dexdump {
public:
    inline static uint8_t GetDexOp(api::MemoryRef& ref) { return ref.value8Of(); }
    inline static bool IsVaildDexOp(uint8_t op) {
        if ((op >= 0x3E && op <= 0x43)
                || op == 0x73
                || (op >= 0x79 && op <= 0x7A)
                || (op >= 0xE3 && op <= 0xF9)) {
            return false;
        }
        return true;
    }
    static uint32_t GetDexInstSize(api::MemoryRef& ref);
    static std::string PrettyDexInst(api::MemoryRef& ref, DexFile& dex_file);
    static void AppendCodecNoArgs(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecNop(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecReturnVoid(api::MemoryRef& ref, std::string& sb);
};

} // namespace art

#endif  // ANDROID_ART_DEXDUMP_DEXDUMP_H_
