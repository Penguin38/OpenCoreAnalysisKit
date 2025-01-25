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

#include "fdtrack/fdtrack.h"
#include "unwindstack/Unwinder.h"
#include "api/core.h"
#include "api/elf.h"
#include "cxx/vector.h"
#include "cxx/mutex.h"
#include "common/bit.h"
#include "base/macros.h"
#include <linux/elf.h>

struct FdEntry_OffsetTable __FdEntry_offset__;
struct FdEntry_SizeTable __FdEntry_size__;

namespace android {

void FdTrack::Init() {
    android::FdEntry::Init();
}

const char* FdTrack::GetPath() {
    return (CoreApi::Bits() == 64) ? FDTRACK64 : FDTRACK32;
}

void FdEntry::Init() {
    if (CoreApi::Bits() == 64) {
        __FdEntry_offset__ = {
            .backtrace = 40,
        };

        __FdEntry_size__ = {
            .THIS = 64,
        };
    } else {
        __FdEntry_offset__ = {
            .backtrace = 4,
        };

        __FdEntry_size__ = {
            .THIS = 16,
        };
    }
}

/*
 *  libfdtrack.so                       stack_traces
 *    --------                    ---  -------------   ---->  -----------
 *   |  LOAD  |                   |   | std::mutex  |  |     | FrameData |
 *   |--------|                   |   |-------------|  |     |---------- |
 *   |  LOAD  |       --------    |   | std::vector | --     | FrameData |
 *   |--------|      |        |   |    -------------         |-----------|
 *   |  LOAD  |  |-->| .data  | <--   |   ......    |        | ...  ...  |
 *   |--------|  |   | .bss   |       |    4096     |        |-----------|
 *   |  LOAD  |---   |        |       |             |        | FrameData |
 *    --------        --------         -------------          -----------
 */
api::MemoryRef FdTrack::AnalysisStackTraces() {
    api::MemoryRef stack_traces = 0x0;
    LinkMap* handle = CoreApi::FindLinkMap(FdTrack::GetPath());
    if (!handle)
        return stack_traces;

    api::Elfx_Ehdr ehdr(handle->l_addr());
    if (!ehdr.IsElf())
        return stack_traces;

    api::Elfx_Phdr phdr(ehdr.Ptr() + ehdr.e_phoff(), ehdr);
    int phnum = ehdr.e_phnum();

    uint64_t last_load = 0x0;
    uint64_t last_load_end = 0x0;
    api::Elfx_Phdr tmp = phdr;
    int index = 0;
    while (index < phnum) {
        if (tmp.p_type() == PT_LOAD) {
            last_load = RoundDown(handle->l_addr() + tmp.p_vaddr(), CoreApi::GetPageSize());
            last_load_end = last_load + RoundUp(tmp.p_memsz(), tmp.p_align());
        }
        index++;
        tmp.MovePtr(SIZEOF(Elfx_Phdr));
    }

    if (!last_load)
        return stack_traces;

    LOGI(">>> analysis scan:[%" PRIx64 ", %" PRIx64 ")\n", last_load, last_load_end);
    int count = 0;
    FdEntry entry = last_load;
    uint64_t point_size = CoreApi::GetPointSize();
    uint64_t endloop = last_load_end - kFdTableSize * SIZEOF(FdEntry);
    int loopcount = (endloop - entry.Ptr()) / point_size;
    do {
        cxx::vector backtrace = entry.backtrace();
        backtrace.SetEntrySize(SIZEOF(FrameData));
        if (!backtrace.IsValid())
            break;

        uint64_t fd = 0;
        api::MemoryRef __begin = backtrace.__begin();
        __begin.Prepare(false);
        if (!((backtrace.__end() - backtrace.__begin()) % SIZEOF(FrameData))
                && __begin.Block() && (__begin.Block()->flags() & Block::FLAG_W)
                && __begin.Block()->virtualContains(backtrace.__end())
                /*&& __begin.Block()->virtualContains(backtrace.__value())*/) {
            FdEntry tmp = entry;
            do {
                try {
                    fd++;
                    tmp.MovePtr(SIZEOF(FdEntry));
                    backtrace = tmp.backtrace();
                    backtrace.SetEntrySize(SIZEOF(FrameData));
                    __begin = backtrace.__begin();
                    __begin.Prepare(false);
                    if (((backtrace.__end() - backtrace.__begin()) % SIZEOF(FrameData))
                            || !__begin.Block() || !(__begin.Block()->flags() & Block::FLAG_W)
                            || !__begin.Block()->virtualContains(backtrace.__end())
                            /*|| !__begin.Block()->virtualContains(backtrace.__value())*/)
                        throw InvalidAddressException(tmp.Ptr());
                } catch(InvalidAddressException& e) {
                    fd = 0;
                    break;
                }
            } while (fd < kFdTableSize - 1);
        }
        if (fd == kFdTableSize - 1) {
            stack_traces = entry.Ptr() /*- (kFdTableSize - 1) * SIZEOF(FdEntry)*/;
            LOGI(">>> stack_traces = 0x%" PRIx64 "\n", stack_traces.Ptr());
            break;
        }
        count++;
        entry.MovePtr(point_size);
    } while(count < loopcount);
    return stack_traces;
}

api::MemoryRef FdTrack::GetStackTraces() {
    api::MemoryRef stack_traces = CoreApi::DlSym(FdTrack::GetPath(),
                                                 FdTrack::FD_TRACK_STACK_TRACES);
    if (stack_traces.Ptr())
        return stack_traces;

    try {
        stack_traces = AnalysisStackTraces();
    } catch(InvalidAddressException& e) {
        // do nothing
    }
    return stack_traces;
}

} // namespace android;
