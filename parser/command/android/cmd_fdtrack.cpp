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
#include "command/android/cmd_fdtrack.h"
#include "api/core.h"
#include "api/memory_ref.h"
#include "cxx/vector.h"
#include "cxx/string.h"
#include "base/utils.h"
#include "unwindstack/Unwinder.h"
#include "command/core/backtrace/cmd_backtrace.h"
#include <unistd.h>
#include <getopt.h>
#include <unordered_map>

int FdtrackCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return Command::FINISH;

    options.dump_top = false;
    options.top = 5;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"top",    required_argument,  0,  't'},
        {0,        0,                  0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "t:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                options.dump_top = true;
                options.top = std::atoi(optarg);
                break;
        }
    }
    options.optind = optind;

    return Command::ONCHLD;
}

int FdtrackCommand::main(int argc, char* const argv[]) {
    api::MemoryRef stack_traces = android::FdTrack::GetStackTraces();
    if (!stack_traces.Ptr()) {
        LOGE("Can not found \"_ZL12stack_traces\", Please sysroot libfdtrack.so!!\n");
        return 0;
    }

    std::array<std::vector<NativeFrame>, android::FdTrack::kFdTableSize> fdv;
    android::FdEntry entry = stack_traces;
    for (int fd = 0; fd < android::FdTrack::kFdTableSize; ++fd) {
        cxx::vector backtrace = entry.backtrace();
        backtrace.SetEntrySize(SIZEOF(FrameData));
        entry.MovePtr(SIZEOF(FdEntry));

        uint32_t frameid = 0;
        std::vector<NativeFrame> nfv;
        for (const auto& value : backtrace) {
            NativeFrame nf;
            android::UnwindStack::FrameData frame = value;
            nf.id = frameid;
            nf.offset = frame.function_offset();
            nf.pc = frame.pc();
            nf.method = frame.GetMethod();
            nfv.push_back(nf);
            ++frameid;
        }
        fdv[fd] = nfv;
    }

    if (options.optind < argc) {
        int fd = std::atoi(argv[options.optind]);
        if (fd < 0 || fd >= android::FdTrack::kFdTableSize) {
            LOGE("unknown fd\n");
            return 0;
        }
        LOGI("fd %d:\n", fd);
        std::vector<NativeFrame>& nfv = fdv[fd];
        FdtrackCommand::ShowStack(nfv);
    } else {
        if (!options.dump_top) {
            for (int fd = 0; fd < android::FdTrack::kFdTableSize; ++fd) {
                if (!fdv[fd].size())
                    continue;

                LOGI("fd %d:\n", fd);
                std::vector<NativeFrame>& nfv = fdv[fd];
                FdtrackCommand::ShowStack(nfv);
            }
        } else {
            FdtrackCommand::ShowTopStack(fdv, options.top);
        }
    }
    return 0;
}

void FdtrackCommand::ShowStack(std::vector<NativeFrame>& nfv) {
    std::string format = BacktraceCommand::FormatNativeFrame("  ", nfv.size());
    for (const auto& frame : nfv) {
        std::string method_desc = frame.method;
        uint64_t offset = frame.offset;
        if (offset) method_desc.append("+").append(Utils::ToHex(offset));
        LOGI(format.c_str(), frame.id, frame.pc, method_desc.c_str());
    }
}

void FdtrackCommand::ShowTopStack(std::array<std::vector<NativeFrame>, android::FdTrack::kFdTableSize> fdv, uint32_t num) {
    std::unordered_map<uint32_t, uint32_t> cloc_map;
    std::unordered_map<uint32_t, std::vector<NativeFrame>> cloc_frame;
    for (int fd = 0; fd < android::FdTrack::kFdTableSize; ++fd) {
        if (!fdv[fd].size())
            continue;

        std::array<uint64_t, android::FdTrack::kStackDepth * 4> fdpcs;
        std::vector<NativeFrame>& nfv = fdv[fd];
        int depth = 0;
        for (const auto& frame : nfv) {
            fdpcs[depth++] = frame.pc;
            if (depth == android::FdTrack::kStackDepth * 4)
                break;
        }

        uint32_t crc32 = Utils::CRC32((uint8_t *)fdpcs.data(), depth * 8);
        if (cloc_map[crc32] == 0)
            cloc_frame[crc32] = nfv;
        cloc_map[crc32] += 1;
    }

    std::vector<SortKey> sort_frame;
    for (int i = 0; i < num; ++i) {
        uint32_t max_crc32 = 0;
        uint32_t max_count = 0;
        for (const auto& value : cloc_map) {
            if (value.second > max_count) {
                max_crc32 = value.first;
                max_count = value.second;
            }
        }
        if (max_crc32) {
            SortKey key = {
                .crc32 = max_crc32,
                .count = max_count,
            };
            sort_frame.push_back(key);
            cloc_map.erase(key.crc32);
        }
    }

    for (int i = 0; i < sort_frame.size(); ++i) {
        LOGI("CRC32[%x]  COUNT[%d]\n", sort_frame[i].crc32, sort_frame[i].count);
        FdtrackCommand::ShowStack(cloc_frame[sort_frame[i].crc32]);
    }
}

void FdtrackCommand::usage() {
    LOGI("Usage: fdtrack [<FD>] [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    -t, --top <NUM>  collect top stack\n");
    ENTER();
    LOGI("core-parser> fdtrack --top 1\n");
    LOGI("CRC32[7c8101a9]  COUNT[1]\n");
    LOGI("  Native: #00  00000074682b7b90  socketpair+0x184\n");
    LOGI("  Native: #01  00000074684fdfac  android::InputChannel::openInputChannelPair(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&, std::__1::unique_ptr<android::InputChannel, std::__1::default_delete<android::InputChannel> >&, std::__1::unique_ptr<android::InputChannel, std::__1::default_delete<android::InputChannel> >&)+0x54\n");
    LOGI("  Native: #02  0000007123af75d4  android::inputdispatcher::InputDispatcher::createInputChannel(std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> > const&)+0x54\n");
    LOGI("  Native: #03  00000071bdc8be54  android::nativeCreateInputChannel(_JNIEnv*, _jobject*, _jstring*)+0x184\n");
    LOGI("  Native: #04  0000000071c610ec  art_jni_trampoline+0x7c\n");
    LOGI("  Native: #05  00000071c2a0b190  nterp_helper+0x1e20\n");
    LOGI("  Native: #06  00000071af03c0e4  com.android.server.input.InputManagerService.createInputChannel+0x4\n");
    LOGI("  Native: #07  00000071c2a0a2c4  nterp_helper+0xf54\n");
    LOGI("  Native: #08  00000071ae9d874c  com.android.server.wm.WindowState.openInputChannel+0x18\n");
    LOGI("  ...\n");
}
