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

#include "logger/log.h"
#include "api/core.h"
#include "api/unwind.h"
#include "arm64/unwind.h"
#include "base/utils.h"
#include "common/elf.h"
#include "common/exception.h"
#include "command/env.h"
#include "command/core/backtrace/cmd_backtrace.h"
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
    dump_fps.clear();

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"all",    no_argument,       0,  'a'},
        {"detail", no_argument,       0,  'd'},
        {"fp",     required_argument, 0,  'f'},
    };

    while ((opt = getopt_long(argc, (char* const*)argv, "adf:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                dump_all = true;
                break;
            case 'd':
                dump_detail = true;
                break;
            case 'f': {
                std::unique_ptr<char> newpath(strdup(optarg));
                char *token = strtok(newpath.get(), ":");
                while (token != nullptr) {
                    dump_fps.push_back(Utils::atol(token));
                    token = strtok(nullptr, ":");
                }
            } break;
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
                addThread(std::atoi(argv[i]));
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
            } catch(InvalidAddressException& e) {}
        } else {
            LOGI("Thread(\"" ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "\") " ANSI_COLOR_CYAN "%s\n" ANSI_COLOR_RESET,
                    record->pid, art::Runtime::Current().Ptr() ? "NotAttachJVM" : "");
        }
#else
        LOGI("Thread(\"" ANSI_COLOR_YELLOW "%d" ANSI_COLOR_RESET "\")\n", record->pid);
#endif
        DumpNativeStack(record->thread, record->api);
#if defined(__AOSP_PARSER__)
        try {
            DumpJavaStack(record->thread, record->api);
        } catch(InvalidAddressException& e) {
            LOGI(ANSI_COLOR_RED "  (STACK MAYBE INCOIMPLETE)\n" ANSI_COLOR_RESET);
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
            uint64_t offset = (native_frame->GetFramePc() & CoreApi::GetVabitsMask()) - native_frame->GetMethodOffset();
            if (offset && native_frame->GetMethodOffset())
                method_desc.append("+").append(Utils::ToHex(offset));

            if (!method_desc.length() && native_frame->GetLinkMap()
                    && native_frame->GetLinkMap()->begin()) {
                method_desc.append(native_frame->GetLibrary());
                method_desc.append("+").append(Utils::ToHex(offset-native_frame->GetLinkMap()->begin()));
            }
            LOGI(format.c_str(), frameid, native_frame->GetFramePc(), method_desc.c_str());
            ++frameid;
            if (frameid == unwind_stack->GetContextNum()) {
                LOGI(ANSI_COLOR_LIGHTRED "    <<maybe handle signal ucontext: 0x%" PRIx64 ">>\n" ANSI_COLOR_RESET, unwind_stack->GetContext());
                unwind_stack->DumpContextRegister("  ");
            }
        }
    }
}

static void PrintObject(art::mirror::Object& obj, const char* msg, uint32_t owner_tid) {
    std::string msg_buf;
    msg_buf.append(msg);
    if (!obj.Ptr() || !obj.IsValid()) {
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

void BacktraceCommand::DumpJavaStack(void *th, ThreadApi* api) {
    if (!th) return;

    art::Thread* thread = reinterpret_cast<art::Thread*>(th);
    art::StackVisitor visitor(thread, art::StackVisitor::StackWalkKind::kSkipInlinedFrames);
    visitor.WalkStack();

    std::string format = FormatJavaFrame("  ", visitor.GetJavaFrames().size());
    uint32_t frameid = 0;
    uint32_t subjni = 0;
    for (const auto& java_frame : visitor.GetJavaFrames()) {
        art::QuickFrame& prev_quick_frame = java_frame->GetPrevQuickFrame();
        if (dump_detail && (prev_quick_frame.Ptr() && prev_quick_frame.GetMethod().IsRuntimeMethod()
                || java_frame->GetMethod().IsNative())) {
            if (frameid) {
                LOGI(ANSI_COLOR_LIGHTRED "    <<JNI INTERFACE CALL JAVA METHOD>>\n" ANSI_COLOR_RESET);
                if (CoreApi::GetMachine() == EM_AARCH64 && dump_fps.size() > subjni) {
                    std::unique_ptr<arm64::UnwindStack> unwind_stack = std::make_unique<arm64::UnwindStack>(api);
                    unwind_stack->OnlyFpBackStack(dump_fps[subjni]);
                    std::string sub_format = FormatJNINativeFrame("      ", unwind_stack->GetNativeFrames().size());
                    uint32_t sub_frameid = 0;
                    for (const auto& native_frame : unwind_stack->GetNativeFrames()) {
                        std::string method_desc = native_frame->GetMethodName();
                        uint64_t offset = (native_frame->GetFramePc() & CoreApi::GetVabitsMask()) - native_frame->GetMethodOffset();
                        if (offset && native_frame->GetMethodOffset())
                            method_desc.append("+").append(Utils::ToHex(offset));

                        if (!method_desc.length() && native_frame->GetLinkMap()
                                && native_frame->GetLinkMap()->begin()) {
                            method_desc.append(native_frame->GetLibrary());
                            method_desc.append("+").append(Utils::ToHex(offset-native_frame->GetLinkMap()->begin()));
                        }
                        LOGI(sub_format.c_str(), sub_frameid, native_frame->GetFramePc(), method_desc.c_str());
                        ++sub_frameid;
                    }
                }
                subjni++;
            }

            if (java_frame->GetMethod().IsNative()) {
                LOGI(ANSI_COLOR_LIGHTRED "    <<%s.%s>>\n" ANSI_COLOR_RESET,
                    java_frame->GetMethod().GetDeclaringClass().PrettyDescriptor().c_str(), java_frame->GetMethod().GetName());
            } else {
                art::ArtMethod& prev_method = prev_quick_frame.GetMethod();
                LOGI(ANSI_COLOR_LIGHTRED "    %s\n" ANSI_COLOR_RESET, prev_method.GetName());
            }
        }

        LOGI(format.c_str(), frameid, java_frame->GetDexPcPtr(),
             dump_detail ? java_frame->GetMethod().ColorPrettyMethodOnlyNP().c_str()
                         : java_frame->GetMethod().ColorPrettyMethodSimple().c_str());
        if (!frameid) {
            try {
                DumpJavaFrameState("  ", thread, java_frame.get());
            } catch(InvalidAddressException& e) {}
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
    format.append("d  ");
    format.append(Logger::LightCyan());
    format.append("%0");
    num = CoreApi::Bits() / 4;
    format.append(std::to_string(num));
    format.append("" PRIx64 "  ");
    format.append(Logger::LightYellow());
    format.append("%s\n");
    format.append(Logger::End());
    return format;
}

std::string BacktraceCommand::FormatJNINativeFrame(const char* prefix, uint64_t size) {
    std::string format;
    format.append(prefix);
    format.append("JNI: #%0");
    int num = 0;
    uint64_t current = size;
    do {
        current = current / 10;
        ++num;
    } while(current != 0);
    format.append(std::to_string(num));
    format.append("d  ");
    format.append(Logger::Cyan());
    format.append("%0");
    num = CoreApi::Bits() / 4;
    format.append(std::to_string(num));
    format.append("" PRIx64 "  ");
    format.append(Logger::Yellow());
    format.append("%s\n");
    format.append(Logger::End());
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
    format.append("d  ");
    format.append(Logger::Cyan());
    format.append("%0");
    num = CoreApi::Bits() / 4;
    format.append(std::to_string(num));
    format.append("" PRIx64 "  ");
    format.append(Logger::Yellow());
    format.append("%s\n");
    format.append(Logger::End());
    return format;
}

void BacktraceCommand::usage() {
    LOGI("Usage: backtrace|bt [PID..] [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -a, --all           show thread stack.\n");
    LOGI("    -d, --detail        show more info.\n");
    LOGI("        --fp <FP_REG>   only support arm64\n");
    ENTER();
    LOGI("core-parser> bt\n");
    LOGI("\"main\" sysTid=6118 Runnable\n");
    LOGI("  | group=\"main\" daemon=0 prio=5 target=0x0\n");
    LOGI("  | tid=1 sCount=0 flags=0 obj=0x71bdaeb8 self=0x7919cce70380\n");
    LOGI("  | stack=0x7ffc732d1000-0x7ffc732d3000 stackSize=0x800000 handle=0x791af2dde4f8\n");
    LOGI("  | mutexes=0x7919cce70b30 held=\"mutator lock\"(shared held) \n");
    LOGI("  rax 0x0000000000000000  rbx 0x0000000000000000  rcx 0x0000000000000000  rdx 0x0000000000000000  \n");
    LOGI("  r8  0x0000000000000002  r9  0x00007919cce70380  r10 0x0000000000000001  r11 0x0000000000000029  \n");
    LOGI("  r12 0x000079192ce6b090  r13 0x00007919cce70380  r14 0x0000000000000002  r15 0x0000000070897508  \n");
    LOGI("  rdi 0x000079192ce6b090  rsi 0x0000000070d05730  \n");
    LOGI("  rbp 0x000000000000f9e1  rsp 0x00007ffc73acc290  rip 0x000079185c88945f  flags 0x0000000000010246  \n");
    LOGI("  ds 0x00000000  es 0x00000000  fs 0x00000000  gs 0x00000000  cs 0x00000033  ss 0x0000002b\n");
    LOGI("  Native: #0  000079185c88945f  \n");
    LOGI("  JavaKt: #0  000079185af57268  android.os.ThreadLocalWorkSource.getToken\n");
    LOGI("  JavaKt: #1  000079185af57290  android.os.ThreadLocalWorkSource.setUid\n");
    LOGI("  JavaKt: #2  000079185af3fdba  android.os.Looper.loop\n");
    LOGI("  JavaKt: #3  000079185b67d8d6  android.app.ActivityThread.main\n");
    LOGI("  JavaKt: #4  0000000000000000  java.lang.reflect.Method.invoke\n");
    LOGI("  JavaKt: #5  000079185a700626  com.android.internal.os.RuntimeInit$MethodAndArgsCaller.run\n");
    LOGI("  JavaKt: #6  000079185a704980  com.android.internal.os.ZygoteInit.main\n");
}
