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

#include "runtime/thread.h"
#include "runtime/base/locks.h"
#include "runtime/managed_stack.h"
#include "runtime/monitor.h"
#include "android.h"
#include "cxx/string.h"
#include "java/lang/Thread.h"
#include "java/lang/ThreadGroup.h"
#include "common/exception.h"
#include "common/bit.h"
#include <string>

struct Thread_OffsetTable __Thread_offset__;
struct Thread_SizeTable __Thread_size__;
struct Thread_tls_32bit_sized_values_OffsetTable __Thread_tls_32bit_sized_values_offset__;
struct Thread_tls_32bit_sized_values_SizeTable __Thread_tls_32bit_sized_values_size__;
struct Thread_tls_ptr_sized_values_OffsetTable __Thread_tls_ptr_sized_values_offset__;
struct Thread_tls_ptr_sized_values_SizeTable __Thread_tls_ptr_sized_values_size__;

namespace art {

void Thread::Init() {
    Android::RegisterSdkListener(Android::O, art::Thread::Init26);
    Android::RegisterSdkListener(Android::P, art::Thread::Init28);
    Android::RegisterSdkListener(Android::Q, art::Thread::Init29);
    Android::RegisterSdkListener(Android::R, art::Thread::Init30);
    Android::RegisterSdkListener(Android::S, art::Thread::Init31);
    Android::RegisterSdkListener(Android::T, art::Thread::Init33);
    Android::RegisterSdkListener(Android::U, art::Thread::Init34);
    Android::RegisterSdkListener(Android::V, art::Thread::Init35);

    Android::RegisterSdkListener(Android::O, art::Thread::tls_ptr_sized_values::Init26);
    Android::RegisterSdkListener(Android::P, art::Thread::tls_ptr_sized_values::Init28);
    Android::RegisterSdkListener(Android::Q, art::Thread::tls_ptr_sized_values::Init29);
    Android::RegisterSdkListener(Android::R, art::Thread::tls_ptr_sized_values::Init30);
    Android::RegisterSdkListener(Android::T, art::Thread::tls_ptr_sized_values::Init33);
    Android::RegisterSdkListener(Android::U, art::Thread::tls_ptr_sized_values::Init34);
    Android::RegisterSdkListener(Android::V, art::Thread::tls_ptr_sized_values::Init35);

    Android::RegisterSdkListener(Android::O, art::Thread::tls_32bit_sized_values::Init26);
    Android::RegisterSdkListener(Android::S, art::Thread::tls_32bit_sized_values::Init31);
}

void Thread::Init26() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 128,
            .wait_monitor_ = 2448,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 128,
            .wait_monitor_ = 1288,
        };
    }
}

void Thread::Init28() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 136,
            .wait_monitor_ = 2520,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 136,
            .wait_monitor_ = 1328,
        };
    }
}

void Thread::Init29() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 152,
            .wait_monitor_ = 6688,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 152,
            .wait_monitor_ = 3432,
        };
    }
}

void Thread::Init30() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 160,
            .wait_monitor_ = 6752,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 160,
            .wait_monitor_ = 3464,
        };
    }
}

void Thread::Init31() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 152,
            .wait_monitor_ = 6736,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 152,
            .wait_monitor_ = 3448,
        };
    }
}

void Thread::Init33() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 144,
            .wait_monitor_ = 6720,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 144,
            .wait_monitor_ = 3432,
        };
    }
}

void Thread::Init34() {
    if (CoreApi::Bits() == 64) {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 152,
            .wait_monitor_ = 6752,
        };
    } else {
        __Thread_offset__ = {
            .tls32_ = 0,
            .tlsPtr_ = 152,
            .wait_monitor_ = 3464,
        };
    }
}

void Thread::Init35() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 144,
        .wait_monitor_ = 6736,
    };
}

void Thread::tls_32bit_sized_values::Init26() {
    __Thread_tls_32bit_sized_values_offset__ = {
        .state_and_flags = 0,
        .suspend_count = 4,
        .thin_lock_thread_id = 12,
        .tid = 16,
    };
}

void Thread::tls_32bit_sized_values::Init31() {
    __Thread_tls_32bit_sized_values_offset__ = {
        .state_and_flags = 0,
        .suspend_count = 4,
        .thin_lock_thread_id = 8,
        .tid = 12,
    };
}

void Thread::tls_ptr_sized_values::Init26() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 208,
            .pthread_self = 216,
            .held_mutexes = 1768,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .jni_env = 28,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 104,
            .pthread_self = 108,
            .held_mutexes = 884,
        };
    }
}

void Thread::tls_ptr_sized_values::Init28() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 208,
            .pthread_self = 216,
            .held_mutexes = 1776,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .jni_env = 36,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 104,
            .pthread_self = 108,
            .held_mutexes = 888,
        };
    }
}

void Thread::tls_ptr_sized_values::Init29() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 208,
            .pthread_self = 216,
            .held_mutexes = 1792,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .jni_env = 28,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 104,
            .pthread_self = 108,
            .held_mutexes = 896,
        };
    }
}

void Thread::tls_ptr_sized_values::Init30() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 192,
            .pthread_self = 200,
            .held_mutexes = 1808,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .jni_env = 28,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 96,
            .pthread_self = 100,
            .held_mutexes = 904,
        };
    }
}

void Thread::tls_ptr_sized_values::Init33() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 192,
            .pthread_self = 200,
            .held_mutexes = 1792,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .jni_env = 28,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 96,
            .pthread_self = 100,
            .held_mutexes = 896,
        };
    }
}

void Thread::tls_ptr_sized_values::Init34() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 184,
            .pthread_self = 192,
            .held_mutexes = 1808,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .jni_env = 28,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 92,
            .pthread_self = 96,
            .held_mutexes = 904,
        };
    }
}

void Thread::tls_ptr_sized_values::Init35() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .jni_env = 56,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 184,
            .pthread_self = 192,
            .held_mutexes = 1800,
        };
    }
}

Thread::tls_32bit_sized_values& Thread::GetTls32() {
    if (!tls32_cache.Ptr()) {
        tls32_cache = tls32();
        tls32_cache.copyRef(this);
    }
    return tls32_cache;
}

Thread::tls_ptr_sized_values& Thread::GetTlsPtr() {
    if (!tlsPtr_cache.Ptr()) {
        tlsPtr_cache = tlsPtr();
        tlsPtr_cache.copyRef(this);
    }
    return tlsPtr_cache;
}

uint16_t Thread::GetFlags() {
    uint32_t state_and_flags = GetTls32().state_and_flags();
    return (state_and_flags & 0xFFFF);
}

ThreadState Thread::GetState() {
    uint32_t state_and_flags = GetTls32().state_and_flags();
    if (Android::Sdk() < Android::T) {
        return static_cast<ThreadState>((state_and_flags >> 16) & 0xFF);
    } else {
        return static_cast<ThreadState>((state_and_flags >> 24) & 0xFF);
    }
}

const char* Thread::GetStateDescriptor() {
    uint8_t state = static_cast<uint8_t>(GetState());
    if (Android::Sdk() < Android::Q) {
        if (Android::Sdk() <= Android::O_MR1) {
            if (state >= static_cast<uint8_t>(ThreadState::kWaitingForLockInflation))
                state += 2;
        }
        if (state >= static_cast<uint8_t>(ThreadState::kNativeForAbort))
            state += 1;
    }
    switch (static_cast<ThreadState>(state)) {
        case ThreadState::kTerminated:
            return "Terminated";
        case ThreadState::kRunnable:
            return "Runnable";
        case ThreadState::kObsoleteRunnable:
            return "Runnable";
        case ThreadState::kTimedWaiting:
            return "TimedWaiting";
        case ThreadState::kSleeping:
            return "Sleeping";
        case ThreadState::kBlocked:
            return "Blocked";
        case ThreadState::kWaiting:
            return "Waiting";
        case ThreadState::kWaitingForLockInflation:
            return "WaitingForLockInflation";
        case ThreadState::kWaitingForTaskProcessor:
            return "WaitingForTaskProcessor";
        case ThreadState::kWaitingForGcToComplete:
            return "WaitingForGcToComplete";
        case ThreadState::kWaitingForCheckPointsToRun:
            return "WaitingForCheckPointsToRun";
        case ThreadState::kWaitingPerformingGc:
            return "WaitingPerformingGc";
        case ThreadState::kWaitingForDebuggerSend:
            return "WaitingForDebuggerSend";
        case ThreadState::kWaitingForDebuggerToAttach:
            return "WaitingForDebuggerToAttach";
        case ThreadState::kWaitingInMainDebuggerLoop:
            return "WaitingInMainDebuggerLoop";
        case ThreadState::kWaitingForDebuggerSuspension:
            return "WaitingForDebuggerSuspension";
        case ThreadState::kWaitingForJniOnLoad:
            return "WaitingForJniOnLoad";
        case ThreadState::kWaitingForSignalCatcherOutput:
            return "WaitingForSignalCatcherOutput";
        case ThreadState::kWaitingInMainSignalCatcherLoop:
            return "WaitingInMainSignalCatcherLoop";
        case ThreadState::kWaitingForDeoptimization:
            return "WaitingForDeoptimization";
        case ThreadState::kWaitingForMethodTracingStart:
            return "WaitingForMethodTracingStart";
        case ThreadState::kWaitingForVisitObjects:
            return "WaitingForVisitObjects";
        case ThreadState::kWaitingForGetObjectsAllocated:
            return "WaitingForGetObjectsAllocated";
        case ThreadState::kWaitingWeakGcRootRead:
            return "WaitingWeakGcRootRead";
        case ThreadState::kWaitingForGcThreadFlip:
            return "WaitingForGcThreadFlip";
        case ThreadState::kNativeForAbort:
            return "NativeForAbort";
        case ThreadState::kStarting:
            return "Starting";
        case ThreadState::kNative:
            return "Native";
        case ThreadState::kSuspended:
            return "Suspended";
    }
    return "Unknown";
}

uint32_t Thread::GetThreadId() {
    return GetTls32().thin_lock_thread_id();
}

uint32_t Thread::GetTid() {
    return GetTls32().tid();
}

const char* Thread::GetName() {
    api::MemoryRef ref = GetTlsPtr().name();
    if (!ref.Ptr())
        return "<Unknown>";

    if (Android::Sdk() < Android::T) {
        cxx::string name = ref;
        return name.c_str();
    } else {
        return reinterpret_cast<const char*>(ref.Real());
    }
}

api::MemoryRef& Thread::GetWaitMonitor() {
    api::MemoryRef& wait_monitor_ = QUICK_CACHE(wait_monitor);
#if defined(__ART_THREAD_WAIT_MONITOR_PARSER__)
    bool invalid = false;
    Monitor monitor_ = wait_monitor_;
    try {
        BaseMutex monitor_lock_(monitor_.monitor_lock(), wait_monitor_);
        if (monitor_lock_.Ptr() && !monitor_lock_.IsMutex()
                || (monitor_.Ptr() && !monitor_.GetObject().IsValid()))
            invalid = true;
    } catch(InvalidAddressException e) {
        invalid = true;
    }

    if (invalid) {
        LOGD("wait_monitor_ invalid, do analysis ...\n");
        bool found = false;
        int count = 0;
        uint64_t point_size = CoreApi::GetPointSize();
        uint64_t endloop = RoundUp(Ptr(), 0x2000);
        int loopcount = (endloop - Ptr()) / point_size;
        do {
            count++;
            monitor_ = valueOf(OFFSET(Thread, wait_monitor_) + count * point_size);
            monitor_.copyRef(this);
            BaseMutex monitor_lock_(monitor_.monitor_lock(), monitor_);
            try {
                if (!monitor_.Ptr() || (monitor_lock_.IsMutex()
                        && monitor_.GetObject().IsValid())) {
                    LOGD(">>> 'wait_monitor_' = 0x%lx\n", monitor_.Ptr());
                    found = true;
                    break;
                }
            } catch(InvalidAddressException e) {}
        } while(count < loopcount);
        if (found) {
            wait_monitor_ = monitor_;
            wait_monitor_.copyRef(monitor_);
            // __Thread_offset__.wait_monitor_ += count * point_size;
        }
    }
#endif
    return wait_monitor_;
}

mirror::Object Thread::GetMonitorEnterObject() {
    return GetTlsPtr().monitor_enter_object();
}

BaseMutex Thread::GetHeldMutex(uint32_t level) {
    api::MemoryRef held_mutexes(GetTlsPtr().held_mutexes(), GetTlsPtr());
    return held_mutexes.valueOf(CoreApi::GetPointSize() * level);
}

JNIEnvExt& Thread::GetJNIEnv() {
    return GetTlsPtr().GetJNIEnv();
}

bool Thread::StackEmpty() {
    ManagedStack current_fragment = GetTlsPtr().managed_stack();
    current_fragment.copyRef(this);

    return current_fragment.GetTopShadowFrame().Ptr() == 0x0 &&
           current_fragment.link() == 0x0 &&
           current_fragment.GetTopQuickFrame().Ptr() == 0x0;
}

void Thread::DumpState() {
    LOGI("\"" ANSI_COLOR_LIGHTRED "%s" ANSI_COLOR_RESET "\" "
              ANSI_COLOR_LIGHTYELLOW "sysTid=%d" ANSI_COLOR_RESET " "
              ANSI_COLOR_LIGHTCYAN "%s\n" ANSI_COLOR_RESET,
         GetName(), GetTid(), GetStateDescriptor());
    java::lang::Thread self = GetTlsPtr().opeer();
    if (self.IsValid()) {
        java::lang::ThreadGroup& group = self.getGroup();
        LOGI(ANSI_COLOR_BLUE "  | group=\"%s\" daemon=%d prio=%d target=0x%x uncaught_exception=0x%x\n" ANSI_COLOR_RESET,
                group.Ptr() ? group.Name().c_str() : "<unknown>", self.getDaemon(),
                self.getPriority(), self.getTarget().Ptr(), self.getUncaughtExceptionHandler().Ptr());
    }
    LOGI(ANSI_COLOR_BLUE "  | tid=%d sCount=%d flags=%d obj=0x%lx self=0x%lx env=0x%lx\n" ANSI_COLOR_RESET,
            GetThreadId(), GetTls32().suspend_count(),
            GetFlags(), GetTlsPtr().opeer(), Ptr(), GetTlsPtr().jni_env());
    LOGI(ANSI_COLOR_BLUE "  | stack=0x%lx-0x%lx stackSize=0x%lx handle=0x%lx\n" ANSI_COLOR_RESET,
            GetTlsPtr().stack_begin(), GetTlsPtr().stack_end(),
            GetTlsPtr().stack_size(), GetTlsPtr().pthread_self());
    std::string mutexes;
    for (uint8_t i = 0; i < LockLevel::kLockLevelCount; ++i) {
        if (i == LockLevel::kMonitorLock)
            continue;
        BaseMutex mutex = GetHeldMutex(i);
        if (mutex.Ptr()) {
            mutexes.append("\"").append(mutex.GetName()).append("\"");;
            if (mutex.IsReaderWriterMutex()) {
                ReaderWriterMutex rw_mutex = mutex;
                if (rw_mutex.GetExclusiveOwnerTid() == GetTid()) {
                    mutexes.append("(exclusive held)");
                } else {
                    mutexes.append("(shared held)");
                }
            }
            mutexes.append(" ");
        }
    }
    LOGI(ANSI_COLOR_BLUE "  | mutexes=0x%lx held=" ANSI_COLOR_LIGHTRED "%s\n" ANSI_COLOR_RESET, GetTlsPtr().held_mutexes(), mutexes.c_str());
}

} //namespace art
