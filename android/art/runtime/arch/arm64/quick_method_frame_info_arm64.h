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

#ifndef ANDROID_ART_RUNTIME_ARCH_ARM64_QUICK_METHOD_FRAME_INFO_ARM64_H_
#define ANDROID_ART_RUNTIME_ARCH_ARM64_QUICK_METHOD_FRAME_INFO_ARM64_H_

#include <stdint.h>
#include <string>

namespace art {
namespace arm64 {

class Arm64QuickMethodFrameInfo {
public:
    static std::string PrettySpillMask(uint32_t mask);
    static void DumpCoreSpill(const char* prefix, uint64_t sp, uint32_t core_mask, uint32_t frame_size);
};

}  // namespace arm64
}  // namespace art

#endif  // ANDROID_ART_RUNTIME_ARCH_ARM64_QUICK_METHOD_FRAME_INFO_ARM64_H_
