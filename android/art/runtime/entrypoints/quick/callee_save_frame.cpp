/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License";
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

#include "runtime/entrypoints/quick/callee_save_frame.h"

namespace art {

QuickMethodFrameInfo RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType type) {
    QuickMethodFrameInfo frame_info(0, 0, 0);
    return frame_info;
}

uint32_t RuntimeCalleeSaveFrame::GetFpSpills(CalleeSaveType type) {
    return 0x0;
}

uint32_t RuntimeCalleeSaveFrame::GetCoreSpills(CalleeSaveType type) {
    return 0x0;
}

} // namespace art
