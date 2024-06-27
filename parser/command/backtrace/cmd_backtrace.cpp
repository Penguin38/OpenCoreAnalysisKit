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

#include "logger/log.h"
#include "api/core.h"
#include "api/unwind.h"
#include "base/utils.h"
#include "common/elf.h"
#include "common/exception.h"
#include "command/env.h"
#include "command/backtrace/cmd_backtrace.h"
#include "runtime/thread_list.h"
#include "runtime/stack.h"
#include "runtime/monitor.h"
#include "android.h"
#include <unistd.h>
#include <getopt.h>
#include <memory>

int BacktraceCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int pid = Env::CurrentPid();
    dump_all = false;
    dump_detail = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"all",    no_argument,       0,  'a'},
        {"detail", no_argument,       0,  'd'},
    };

    while ((opt = getopt_long(argc, (char* const*)argv, "ad",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                dump_all = true;
                break;
            case 'd':
                dump_detail = true;
                break;
        }
    }

    if (dump_all) {
        auto callback = [&](ThreadApi *api) -> bool {
            std::unique_ptr<BacktraceCommand::ThreadRecord> thread = std::make_unique<BacktraceCommand::ThreadRecord>(api);
            threads.push_back(std::move(thread));
            return false;
        };
        CoreApi::ForeachThread(callback);
#if defined(__AOSP_PARSER__)
        if (Android::IsSdkReady() && art::Runtime::Current().Ptr()) {
            art::ThreadList& thread_list = art::Runtime::Current().GetThreadList();
            for (const auto& thread : thread_list.GetList()) {
                int tid = thread->GetTid();
                addThread(tid, BacktraceCommand::ThreadRecord::TYPE_JVM, thread.get());
            }
        }
#endif
    } else {
        if (optind < argc) {
            for (int i = optind; i < argc; ++i) {
                addThread(atoi(argv[i]));
            }
        } else {
            addThread(pid);
        }
#if defined(__AOSP_PARSER__)
        if (Android::IsSdkReady() && art::Runtime::Current().Ptr()) {
            art::ThreadList& thread_list = art::Runtime::Current().GetThreadList();
            for (const auto& thread : thread_list.GetList()) {
                int tid = thread->GetTid();
                BacktraceCommand::ThreadRecord* record = findRecord(tid);
                if (record) {
                    record->type = BacktraceCommand::ThreadRecord::TYPE_JVM;
                    record->thread = thread.get();
                }
            }
        }
#endif
    }

    DumpTrace();
    threads.clear();
    return 0;
}

void BacktraceCommand::addThread(int pid) {
    addThread(pid, BacktraceCommand::ThreadRecord::TYPE_NATIVE);
}

void BacktraceCommand::addThread(int pid, int type) {
    addThread(pid, type, nullptr);
}

void BacktraceCommand::addThread(int pid, int type, void* at) {
    BacktraceCommand::ThreadRecord* record = findRecord(pid);
    if (!record) {
        ThreadApi* api = CoreApi::FindThread(pid);
        if (api) {
            std::unique_ptr<BacktraceCommand::ThreadRecord> thread = std::make_unique<BacktraceCommand::ThreadRecord>(api, type);
            thread->thread = at;
            threads.push_back(std::move(thread));
        } else {
            std::unique_ptr<BacktraceCommand::ThreadRecord> thread = std::make_unique<BacktraceCommand::ThreadRecord>(pid, type);
            thread->thread = at;
            threads.push_back(std::move(thread));
        }
    } else {
        record->type = type;
        record->thread = at;
    }
}

BacktraceCommand::ThreadRecord* BacktraceCommand::findRecord(int pid) {
    for (const auto& record : threads) {
        if (pid == record->pid)
            return record.get();
    }
    return nullptr;
}

void BacktraceCommand::DumpTrace() {
    bool needEnd = false;
    for (const auto& record : threads) {
        if (needEnd) ENTER();
#if defined(__AOSP_PARSER__)
        if (record->thread) {
            try {
                art::Thread* thread = reinterpret_cast<art::Thread*>(record->thread);
                thread->DumpState();
            } catch(InvalidAddressException e) {}
        } else {
            LOGI("Thread(\"%d\") %s\n", record->pid, art::Runtime::Current().Ptr() ? "NotAttachJVM" : "");
        }
#else
        LOGI("Thread(\"%d\")\n", record->pid);
#endif
        DumpNativeStack(record->thread, record->api);
#if defined(__AOSP_PARSER__)
        try {
            DumpJavaStack(record->thread);
        } catch(InvalidAddressException e) {
            LOGI("  (STACK MAYBE INCOIMPLETE)\n");
        }
#endif
        needEnd = true;
    }
}

void BacktraceCommand::DumpNativeStack(void *thread, ThreadApi* api) {
    if (!api) {
        LOGI("  (NOT EXIST THREAD)\n");
        return;
    }
    api->RegisterDump("  ");
    std::unique_ptr<api::UnwindStack> unwind_stack = api::UnwindStack::MakeUnwindStack(api);
    if (unwind_stack) {
        unwind_stack->WalkStack();
        std::string format = FormatNativeFrame("  ", unwind_stack->GetNativeFrames().size());
        uint32_t frameid = 0;
        for (const auto& native_frame : unwind_stack->GetNativeFrames()) {
            std::string method_desc = native_frame->GetMethodName();
            if (native_frame->GetMethodOffset()) method_desc.append("+").append(Utils::ToHex(native_frame->GetMethodOffset()));
            LOGI(format.c_str(), frameid, native_frame->GetFramePc(), method_desc.c_str());
            ++frameid;
            if (frameid == unwind_stack->GetContextNum()) {
                LOGI("  <<maybe handle signal>>\n");
                unwind_stack->DumpContextRegister("  ");
            }
        }
    }
}

static void PrintObject(art::mirror::Object& obj, const char* msg, uint32_t owner_tid) {
    std::string msg_buf;
    msg_buf.append(msg);
    if (!obj.Ptr()) {
        msg_buf.append("an unknown object");
    } else {
        msg_buf.append("<");
        msg_buf.append(Utils::ToHex(obj.Ptr()));
        msg_buf.append("> (");
        if (obj.IsClass()) {
            msg_buf.append("a java.lang.Class<");
            art::mirror::Class klass = obj;
            msg_buf.append(klass.PrettyDescriptor());
            msg_buf.append(">");
        } else {
            art::mirror::Class klass = obj.GetClass();
            msg_buf.append("a ");
            msg_buf.append(klass.PrettyDescriptor());
        }
        msg_buf.append(")");
    }

    if (owner_tid != art::ThreadList::kInvalidThreadId) {
        art::Runtime& runtime = art::Runtime::Current();
        art::Thread* owner = runtime.GetThreadList().FindThreadByThreadId(owner_tid);
        msg_buf.append(" held by thread ");
        if (owner) {
            msg_buf.append("sysTid=");
            msg_buf.append(std::to_string(owner->GetTid()));
        } else {
            msg_buf.append("tid=");
            msg_buf.append(std::to_string(owner_tid));
        }
    }
    LOGI("%s\n", msg_buf.c_str());
}

static void VisitWaitingObject(art::mirror::Object& obj, art::ThreadState) {
    PrintObject(obj, "  - waiting on ", art::ThreadList::kInvalidThreadId);
}

static void VisitSleepingObject(art::mirror::Object& obj) {
    PrintObject(obj, "  - sleeping on ", art::ThreadList::kInvalidThreadId);
}

static void VisitBlockedOnObject(art::mirror::Object& obj, art::ThreadState state, uint32_t owner_tid) {
    std::string msg;
    switch (state) {
        case art::ThreadState::kBlocked:
            msg.append("  - waiting to lock ");
            break;

        case art::ThreadState::kWaitingForLockInflation:
            msg.append("  - waiting for lock inflation of ");
            break;
        default:
            break;
    }
    PrintObject(obj, msg.c_str(), owner_tid);
}

static void DumpJavaFrameState(const char* prefix, art::Thread* thread, art::JavaFrame* java_frame) {
    art::mirror::Object monitor_object = 0x0;
    uint32_t lock_owner_tid;
    art::ThreadState state = art::Monitor::FetchState(thread, &monitor_object, &lock_owner_tid);
    switch (state) {
        case art::ThreadState::kWaiting:
        case art::ThreadState::kTimedWaiting:
            VisitWaitingObject(monitor_object, state);
            break;
        case art::ThreadState::kSleeping:
            VisitSleepingObject(monitor_object);
            break;

        case art::ThreadState::kBlocked:
        case art::ThreadState::kWaitingForLockInflation:
            VisitBlockedOnObject(monitor_object, state, lock_owner_tid);
            break;
        default:
            break;
    }
}

void BacktraceCommand::DumpJavaStack(void *th) {
    if (!th) return;

    art::Thread* thread = reinterpret_cast<art::Thread*>(th);
    art::StackVisitor visitor(thread, art::StackVisitor::StackWalkKind::kSkipInlinedFrames);
    visitor.WalkStack();

    std::string format = FormatJavaFrame("  ", visitor.GetJavaFrames().size());
    uint32_t frameid = 0;
    for (const auto& java_frame : visitor.GetJavaFrames()) {
        LOGI(format.c_str(), frameid, java_frame->GetDexPcPtr(),
             dump_detail ? java_frame->GetMethod().PrettyMethodOnlyNP().c_str()
                         : java_frame->GetMethod().PrettyMethodSimple().c_str());
        if (!frameid) {
            try {
                DumpJavaFrameState("  ", thread, java_frame.get());
            } catch(InvalidAddressException e) {}
        }
        ++frameid;
    }
}

std::string BacktraceCommand::FormatJavaFrame(const char* prefix, uint64_t size) {
    std::string format;
    format.append(prefix);
    format.append("JavaKt: #%0");
    int num = 0;
    uint64_t current = size;
    do {
        current = current / 10;
        ++num;
    } while(current != 0);
    format.append(std::to_string(num));
    format.append("d  %0");
    num = CoreApi::Bits() / 4;
    format.append(std::to_string(num));
    format.append("lx  %s\n");
    return format;
}

std::string BacktraceCommand::FormatNativeFrame(const char* prefix, uint64_t size) {
    std::string format;
    format.append(prefix);
    format.append("Native: #%0");
    int num = 0;
    uint64_t current = size;
    do {
        current = current / 10;
        ++num;
    } while(current != 0);
    format.append(std::to_string(num));
    format.append("d  %0");
    num = CoreApi::Bits() / 4;
    format.append(std::to_string(num));
    format.append("lx  %s\n");
    return format;
}

void BacktraceCommand::usage() {
    LOGI("Usage: backtrace|bt [PID..] [option..]\n");
    LOGI("Option:\n");
    LOGI("    --all|-a: show thread stack.\n");
    LOGI("    --detail|-d: show more info.\n");
}
