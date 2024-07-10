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

#ifndef CORE_COMMON_NATIVE_FRAME_H_
#define CORE_COMMON_NATIVE_FRAME_H_

#include "common/link_map.h"

class NativeFrame {
public:
    NativeFrame(uint64_t fp, uint64_t sp, uint64_t pc);
    void Decode();
    uint64_t GetFrameFp() { return frame_fp; }
    void SetFramePc(uint64_t pc);
    uint64_t GetFramePc() { return frame_pc; }
    std::string& GetMethodName() { return frame_symbol.GetMethod(); }
    uint64_t GetMethodOffset();
    std::string GetLibrary();
    bool IsThumbMode();
    void SetThumbMode() { thumb = true; }
private:
    uint64_t frame_fp;
    uint64_t frame_sp;
    uint64_t frame_pc;
    LinkMap* map;
    LinkMap::NiceSymbol frame_symbol;
    bool thumb;
};

#endif // CORE_COMMON_NATIVE_FRAME_H_
