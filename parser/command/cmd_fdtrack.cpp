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
#include "command/cmd_fdtrack.h"
#include "api/core.h"
#include "api/memory_ref.h"
#include "cxx/vector.h"
#include "cxx/string.h"
#include "base/utils.h"
#include "fdtrack/fdtrack.h"
#include "unwindstack/Unwinder.h"
#include "command/backtrace/cmd_backtrace.h"

int FdtrackCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    api::MemoryRef stack_traces = CoreApi::DlSym(android::FdTrack::GetPath(),
                                                 android::FdTrack::FD_TRACK_STACK_TRACES);
    if (!stack_traces.Ptr()) {
        LOGE("Can not found \"_ZL12stack_traces\", Please sysroot %s!!\n", android::FdTrack::GetPath());
        return 0;
    }

    android::FdEntry entry = stack_traces;
    for (int fd = 0; fd < android::FdTrack::kFdTableSize; ++fd) {
        cxx::vector backtrace = entry.backtrace();
        backtrace.SetEntrySize(SIZEOF(FrameData));
        entry.MovePtr(SIZEOF(FdEntry));
        if (!backtrace.size())
            continue;

        std::string format = BacktraceCommand::FormatNativeFrame("  ", backtrace.size());
        uint32_t frameid = 0;
        LOGI("fd %d:\n", fd);
        for (const auto& value : backtrace) {
            android::UnwindStack::FrameData frame = value;
            std::string method_desc = frame.GetMethod();
            uint64_t offset = frame.function_offset();
            if (offset) method_desc.append("+").append(Utils::ToHex(offset));
            LOGI(format.c_str(), frameid, frame.pc(), method_desc.c_str());
            ++frameid;
        }
    }
    return 0;
}

void FdtrackCommand::usage() {}
