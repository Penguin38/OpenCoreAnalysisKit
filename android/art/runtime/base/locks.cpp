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

#include "runtime/base/locks.h"
#include "android.h"

namespace art {

uint8_t LockLevel::kMonitorLock = 56;
uint8_t LockLevel::kLockLevelCount = 78;

void LockLevel::Init() {
    Android::RegisterSdkListener(Android::M, art::LockLevel::Init26);
    Android::RegisterSdkListener(Android::N, art::LockLevel::Init26);
    Android::RegisterSdkListener(Android::O, art::LockLevel::Init26);
    Android::RegisterSdkListener(Android::P, art::LockLevel::Init28);
    Android::RegisterSdkListener(Android::Q, art::LockLevel::Init29);
    Android::RegisterSdkListener(Android::R, art::LockLevel::Init30);
}

void LockLevel::Init26() {
    kMonitorLock = 49;
    kLockLevelCount = 64;
}

void LockLevel::Init28() {
    kMonitorLock = 51;
    kLockLevelCount = 70;
}

void LockLevel::Init29() {
    kMonitorLock = 54;
    kLockLevelCount = 75;
}

void LockLevel::Init30() {
    kMonitorLock = 56;
    kLockLevelCount = 78;
}

} // namespace art
