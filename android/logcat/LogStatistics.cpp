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

#include "api/core.h"
#include "logcat/LogStatistics.h"

struct LogStatistics_OffsetTable __LogStatistics_offset__;

namespace android {

void LogStatistics::Init() {
    if (CoreApi::Bits() == 64) {
        __LogStatistics_offset__ = {
            .mElements = 64,
        };
    } else {
        // do nothing
    }
}

} // namespace android
