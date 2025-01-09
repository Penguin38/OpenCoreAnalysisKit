/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License";
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

#ifndef ANDROID_ART_RUNTIME_ENTRYPOINTS_QUICK_CALLEE_SAVE_FRAME_H_
#define ANDROID_ART_RUNTIME_ENTRYPOINTS_QUICK_CALLEE_SAVE_FRAME_H_

#include "runtime/base/callee_save_type.h"
#include "runtime/quick/quick_method_frame_info.h"

namespace art {

class RuntimeCalleeSaveFrame {
public:
    static QuickMethodFrameInfo GetMethodFrameInfo(CalleeSaveType type);
    static uint32_t GetFpSpills(CalleeSaveType type);
    static uint32_t GetCoreSpills(CalleeSaveType type);
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_ENTRYPOINTS_QUICK_CALLEE_SAVE_FRAME_H_
