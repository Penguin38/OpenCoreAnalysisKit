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
#include "common/elf.h"
#include "command/env.h"
#include "command/backtrace/cmd_backtrace.h"
#include <unistd.h>
#include <getopt.h>

#if defined(__AOSP_PARSER__)
#include "runtime/thread_list.h"
#include "runtime/stack.h"
#endif

int BacktraceCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int pid = Env::CurrentPid();
    dump_all = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"all",    no_argument,       0,  'a'},
    };

    while ((opt = getopt_long(argc, (char* const*)argv, "a",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                dump_all = true;
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
        if (Android::IsSdkReady()) {
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
        if (Android::IsSdkReady()) {
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
        if (needEnd) LOGI("\n");
#if defined(__AOSP_PARSER__)
        if (record->thread) {
            art::Thread* thread = reinterpret_cast<art::Thread*>(record->thread);
            thread->DumpState();
        } else {
            LOGI("Thread(\"%d\") NotAttachJVM\n", record->pid);
        }
#else
        LOGI("Thread(\"%d\")\n", record->pid);
#endif
        if (record->api) {
            record->api->RegisterDump("  ");
            //do native stackwalk
        } else {
            LOGI("  (NOT EXIST THREAD)\n");
        }

#if defined(__AOSP_PARSER__)
        if (record->thread) {
            art::Thread* thread = reinterpret_cast<art::Thread*>(record->thread);
            art::StackVisitor visitor(thread, art::StackVisitor::StackWalkKind::kSkipInlinedFrames);
            visitor.WalkStack();
        }
#endif
        needEnd = true;
    }
}

void BacktraceCommand::usage() {
}
