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

#include "api/core.h"
#include "android.h"
#include "runtime/monitor.h"
#include "runtime/lock_word.h"

struct Monitor_OffsetTable __Monitor_offset__;

namespace art {

void Monitor::Init() {
    Android::RegisterSdkListener(Android::M, art::Monitor::Init23);
    Android::RegisterSdkListener(Android::N, art::Monitor::Init24);
    Android::RegisterSdkListener(Android::O, art::Monitor::Init26);
    Android::RegisterSdkListener(Android::P, art::Monitor::Init28);
    Android::RegisterSdkListener(Android::Q, art::Monitor::Init29);
    Android::RegisterSdkListener(Android::R, art::Monitor::Init30);
}

void Monitor::Init23() {
    if (CoreApi::Bits() == 64) {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 96,
            .obj_ = 100,
        };
    } else {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 60,
            .obj_ = 68,
        };
    }
}

void Monitor::Init24() {
    if (CoreApi::Bits() == 64) {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 96,
            .obj_ = 100,
        };
    } else {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 60,
            .obj_ = 68,
        };
    }
}

void Monitor::Init26() {
    if (CoreApi::Bits() == 64) {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 96,
            .obj_ = 108,
        };
    } else {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 60,
            .obj_ = 68,
        };
    }
}

void Monitor::Init28() {
    if (CoreApi::Bits() == 64) {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 88,
            .obj_ = 100,
        };
    } else {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 60,
            .obj_ = 68,
        };
    }
}

void Monitor::Init29() {
    if (CoreApi::Bits() == 64) {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 72,
            .obj_ = 84,
        };
    } else {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 52,
            .obj_ = 60,
        };
    }
}

void Monitor::Init30() {
    if (CoreApi::Bits() == 64) {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 48,
            .obj_ = 60,
        };
    } else {
        __Monitor_offset__ = {
            .monitor_lock_ = 0,
            .owner_ = 36,
            .obj_ = 44,
        };
    }
}

ThreadState Monitor::FetchState(Thread* thread, mirror::Object* monitor_object, uint32_t* lock_owner_tid) {
    *monitor_object = 0x0;
    *lock_owner_tid = ThreadList::kInvalidThreadId;

    ThreadState state = thread->GetState();
    switch (state) {
        case ThreadState::kWaiting:
        case ThreadState::kTimedWaiting:
        case ThreadState::kSleeping:
        {
            Monitor monitor = thread->GetWaitMonitor();
            if (monitor.Ptr()) {
                *monitor_object = monitor.GetObject();
            }
        }
        break;

        case ThreadState::kBlocked:
        case ThreadState::kWaitingForLockInflation:
        {
            mirror::Object lock_object = thread->GetMonitorEnterObject();
            if (lock_object.Ptr()) {
                *monitor_object = lock_object;
                *lock_owner_tid = lock_object.GetLockOwnerThreadId();
            }
        }
        break;

        default:
            break;
    }
    return state;
}

uint32_t Monitor::GetLockOwnerThreadId(mirror::Object& obj) {
    LockWord lock_word = obj.GetLockWord();
    switch (static_cast<uint32_t>(lock_word.GetState())) {
        case LockWord::kHashCode:
            // Fall-through.
        case LockWord::kUnlocked:
            return ThreadList::kInvalidThreadId;
        case LockWord::kThinLocked:
            return lock_word.ThinLockOwner();
        case LockWord::kFatLocked: {
            Monitor monitor = lock_word.FatLockMonitor();
            return monitor.GetOwnerThreadId();
        }
    }
    return ThreadList::kInvalidThreadId;
}

} // namespace art
