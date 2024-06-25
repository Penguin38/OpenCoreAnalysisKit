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

#ifndef ANDROID_ART_RUNTIME_BASE_LOCKS_H_
#define ANDROID_ART_RUNTIME_BASE_LOCKS_H_

#include <stdint.h>

namespace art {

class LockLevel {
public:
    static uint8_t kMonitorLock;
    static uint8_t kLockLevelCount;
    static void Init26();
    static void Init28();
    static void Init29();
    static void Init30();
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_BASE_LOCKS_H_
