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
#include "android.h"
#include "cxx/string.h"
#include "java/lang/Thread.h"
#include "java/lang/ThreadGroup.h"

struct Thread_OffsetTable __Thread_offset__;
struct Thread_SizeTable __Thread_size__;
struct Thread_tls_32bit_sized_values_OffsetTable __Thread_tls_32bit_sized_values_offset__;
struct Thread_tls_32bit_sized_values_SizeTable __Thread_tls_32bit_sized_values_size__;
struct Thread_tls_ptr_sized_values_OffsetTable __Thread_tls_ptr_sized_values_offset__;
struct Thread_tls_ptr_sized_values_SizeTable __Thread_tls_ptr_sized_values_size__;

namespace art {

void Thread::Init26() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 128,
    };
}

void Thread::Init28() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 136,
    };
}

void Thread::Init29() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 152,
    };
}

void Thread::Init30() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 160,
    };
}

void Thread::Init31() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 152,
    };
}

void Thread::Init33() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 144,
    };
}

void Thread::Init34() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 152,
    };
}

void Thread::Init35() {
    __Thread_offset__ = {
        .tls32_ = 0,
        .tlsPtr_ = 144,
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
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 208,
            .pthread_self = 216,
        };
    } else {
        //TODO
    }
}

void Thread::tls_ptr_sized_values::Init30() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 192,
            .pthread_self = 200,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 96,
            .pthread_self = 100,
        };
    }
}

void Thread::tls_ptr_sized_values::Init34() {
    if (CoreApi::Bits() == 64) {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 16,
            .managed_stack = 24,
            .self = 72,
            .opeer = 80,
            .stack_begin = 96,
            .stack_size = 104,
            .monitor_enter_object = 128,
            .name = 184,
            .pthread_self = 192,
        };
    } else {
        __Thread_tls_ptr_sized_values_offset__ = {
            .stack_end = 8,
            .managed_stack = 12,
            .self = 36,
            .opeer = 40,
            .stack_begin = 48,
            .stack_size = 52,
            .monitor_enter_object = 64,
            .name = 92,
            .pthread_self = 96,
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
    if (Android::Sdk() < Android::TIRAMISU) {
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

    if (Android::Sdk() < Android::TIRAMISU) {
        cxx::string name = ref;
        return name.c_str();
    } else {
        return reinterpret_cast<const char*>(ref.Real());
    }
}

void Thread::DumpState() {
    LOGI("\"%s\" tid=%d %s\n", GetName(), GetThreadId(), GetStateDescriptor());
    java::lang::Thread self = GetTlsPtr().opeer();
    if (self.IsValid()) {
        java::lang::ThreadGroup& group = self.getGroup();
        LOGI("  | group=\"%s\" daemon=%d prio=%d target=0x%x\n",
                group.Ptr() ? group.Name().c_str() : "<unknown>", self.getDaemon(),
                self.getPriority(), self.getTarget().Ptr());
    }
    LOGI("  | sysTid=%d sCount=%d flags=%d obj=0x%lx self=0x%lx\n",
            GetTid(), GetTls32().suspend_count(),
            GetFlags(), GetTlsPtr().opeer(), Ptr());
    LOGI("  | stack=0x%lx-0x%lx stackSize=0x%lx handle=0x%lx\n",
            GetTlsPtr().stack_begin(), GetTlsPtr().stack_end(),
            GetTlsPtr().stack_size(), GetTlsPtr().pthread_self());
    LOGI("  | held mutexes=\n");
}

} //namespace art
