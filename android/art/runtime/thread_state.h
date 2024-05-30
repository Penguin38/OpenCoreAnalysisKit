/*
 * Copyright (C) 2013 The Android Open Source Project
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

#ifndef ART_RUNTIME_THREAD_STATE_H_
#define ART_RUNTIME_THREAD_STATE_H_

namespace art {

// State stored in our C++ class Thread.
// When we refer to "a suspended state", or when function names mention "ToSuspended" or
// "FromSuspended", we mean any state other than kRunnable, i.e. any state in which the thread is
// guaranteed not to access the Java heap. The kSuspended state is merely one of these.
enum class ThreadState : uint8_t {
  // `kRunnable` was previously 67 but it is now set to 0 so that we do not need to extract
  // flags from the thread's `state_and_flags` to check for any flag being set while Runnable.
  // Note: All atomic accesses for a location should use the same data size,
  // so the incorrect old approach of reading just 16 bits has been rewritten.

  //                                   Java
  //                                   Thread.State   JDWP state
  kTerminated = 66,                 // TERMINATED     TS_ZOMBIE    Thread.run has returned, but Thread* still around
  kRunnable = 0,                    // RUNNABLE       TS_RUNNING   runnable
  kObsoleteRunnable = 67,           // ---            ---          obsolete value
  kTimedWaiting = 68,               // TIMED_WAITING  TS_WAIT      in Object.wait() with a timeout
  kSleeping,                        // TIMED_WAITING  TS_SLEEPING  in Thread.sleep()
  kBlocked,                         // BLOCKED        TS_MONITOR   blocked on a monitor
  kWaiting,                         // WAITING        TS_WAIT      in Object.wait()
  kWaitingForLockInflation,         // WAITING        TS_WAIT      blocked inflating a thin-lock         //  SDK 23
  kWaitingForTaskProcessor,         // WAITING        TS_WAIT      blocked waiting for taskProcessor     //  sdk 23
  kWaitingForGcToComplete,          // WAITING        TS_WAIT      blocked waiting for GC
  kWaitingForCheckPointsToRun,      // WAITING        TS_WAIT      GC waiting for checkpoints to run
  kWaitingPerformingGc,             // WAITING        TS_WAIT      performing GC
  kWaitingForDebuggerSend,          // WAITING        TS_WAIT      blocked waiting for events to be sent
  kWaitingForDebuggerToAttach,      // WAITING        TS_WAIT      blocked waiting for debugger to attach
  kWaitingInMainDebuggerLoop,       // WAITING        TS_WAIT      blocking/reading/processing debugger events
  kWaitingForDebuggerSuspension,    // WAITING        TS_WAIT      waiting for debugger suspend all
  kWaitingForJniOnLoad,             // WAITING        TS_WAIT      waiting for execution of dlopen and JNI on load code
  kWaitingForSignalCatcherOutput,   // WAITING        TS_WAIT      waiting for signal catcher IO to complete
  kWaitingInMainSignalCatcherLoop,  // WAITING        TS_WAIT      blocking/reading/processing signals
  kWaitingForDeoptimization,        // WAITING        TS_WAIT      waiting for deoptimization suspend all
  kWaitingForMethodTracingStart,    // WAITING        TS_WAIT      waiting for method tracing to start
  kWaitingForVisitObjects,          // WAITING        TS_WAIT      waiting for visiting objects
  kWaitingForGetObjectsAllocated,   // WAITING        TS_WAIT      waiting for getting the number of allocated objects
  kWaitingWeakGcRootRead,           // WAITING        TS_WAIT      waiting on the GC to read a weak root
  kWaitingForGcThreadFlip,          // WAITING        TS_WAIT      waiting on the GC thread flip (CC collector) to finish
  kNativeForAbort,                  // WAITING        TS_WAIT      checking other threads are not run on abort.   //  SDK 29
  kStarting,                        // NEW            TS_WAIT      native thread started, not yet ready to run managed code
  kNative,                          // RUNNABLE       TS_RUNNING   running in a JNI native method
  kSuspended,                       // RUNNABLE       TS_RUNNING   suspended by GC or debugger
};

}  // namespace art

#endif  // ART_RUNTIME_THREAD_STATE_H_
