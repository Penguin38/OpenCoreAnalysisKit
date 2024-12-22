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
#include "command/fake/core/x86_64/tombstone_parser.h"
#include <stdio.h>
#include <string.h>

namespace x86_64 {

static char kLine[1024];

bool TombstoneParser::parse() {
    mCrashTid = 0;
    memset(&regs, 0x0, sizeof(regs));

    bool ret = parseTid() &&
               parseRegister() &&
               parseBacktrace() &&
               parseMemory() &&
               parseMaps();

    LOGI("Tid: %d\n", mCrashTid);
    LOGI("rax 0x%016" PRIx64 "  rbx 0x%016" PRIx64 "  rcx 0x%016" PRIx64 "  rdx 0x%016" PRIx64 "  \n", regs.rax, regs.rbx, regs.rcx, regs.rdx);
    LOGI("r8  0x%016" PRIx64 "  r9  0x%016" PRIx64 "  r10 0x%016" PRIx64 "  r11 0x%016" PRIx64 "  \n", regs.r8, regs.r9, regs.r10, regs.r11);
    LOGI("r12 0x%016" PRIx64 "  r13 0x%016" PRIx64 "  r14 0x%016" PRIx64 "  r15 0x%016" PRIx64 "  \n", regs.r12, regs.r13, regs.r14, regs.r15);
    LOGI("rdi 0x%016" PRIx64 "  rsi 0x%016" PRIx64 "  \n", regs.rdi, regs.rsi);
    LOGI("rbp 0x%016" PRIx64 "  rsp 0x%016" PRIx64 "  rip 0x%016" PRIx64 "  \n", regs.rbp, regs.rsp, regs.rip);

    for (const auto& lib : mLibs)
        LOGI("%s\n", lib.c_str());

    for (const auto& vma : mMaps)
        LOGD("[%" PRIx64 ", %" PRIx64 ") %s\n", vma.begin, vma.end, vma.file.c_str());

    return ret;
}

bool TombstoneParser::parseTid() {
    LOGD("%s ...\n", __func__);
    int pid;
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (sscanf(kLine, "pid: %d, tid: %d", &pid, &mCrashTid))
            return true;
    }
    return false;
}

bool TombstoneParser::parseRegister() {
    LOGD("%s ...\n", __func__);
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (!sscanf(kLine, " rax %" PRIx64 " rbx %" PRIx64 " rcx %" PRIx64 " rdx %" PRIx64 "",
                    &regs.rax, &regs.rbx, &regs.rcx, &regs.rdx))
            continue;

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " r8 %" PRIx64 " r9 %" PRIx64 " r10 %" PRIx64 " r11 %" PRIx64 "", &regs.r8, &regs.r9, &regs.r10, &regs.r11);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " r12 %" PRIx64 " r13 %" PRIx64 " r14 %" PRIx64 " r15 %" PRIx64 "", &regs.r12, &regs.r13, &regs.r14, &regs.r15);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " rdi %" PRIx64 " rsi %" PRIx64 "", &regs.rdi, &regs.rsi);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " rbp %" PRIx64 " rsp %" PRIx64 " rip %" PRIx64 "", &regs.rbp, &regs.rsp, &regs.rip);

        return true;
    }
    return false;
}

bool TombstoneParser::parseBacktrace() {
    LOGD("%s ...\n", __func__);
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (strncmp(kLine, "backtrace:", 10))
            continue;

        int m;
        char filename[256] = {'\0'};
        char buildid[1024] = {'\n'};
        while (fgets(kLine, sizeof(kLine), mFp)) {
            if (!strcmp(kLine, "\n"))
                return true;

            sscanf(kLine, "      #%*d pc %*" PRIx64 "  %s %1023[^\n] %n", filename, buildid, &m);

            if (strlen(filename) && filename[0] == '/') {
                std::string libname_append_buildid = filename;
                if (strlen(buildid)) {
                    std::string buildid_str = buildid;
                    std::size_t l_pos = buildid_str.find(" (BuildId: ");
                    std::size_t r_pos = buildid_str.rfind(')');
                    if (l_pos != std::string::npos && r_pos != std::string::npos) {
                        buildid_str = buildid_str.substr(l_pos + 11, 32);
                        libname_append_buildid.append(":");
                        libname_append_buildid.append(buildid_str);
                    }
                }
                mLibs.insert(libname_append_buildid);
            }
        }
    }
    return false;
}

bool TombstoneParser::parseMemory() {
    LOGD("%s ...\n", __func__);
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (strncmp(kLine, "memory near ", 12)
                && strncmp(kLine, "code around ", 12))
            continue;

        uint64_t addr;
        uint64_t value1;
        uint64_t value2;
        while (fgets(kLine, sizeof(kLine), mFp)) {
            if (!strcmp(kLine, "\n"))
                break;

            if (sscanf(kLine, "    %" PRIx64 " %" PRIx64 " %" PRIx64 "", &addr, &value1, &value2)) {
                mMemorys.insert({addr, value1});
                mMemorys.insert({addr + 8, value2});
            }
        }
    }
    rewind(mFp);
    return true;
}

bool TombstoneParser::parseMaps() {
    LOGD("%s ...\n", __func__);
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (strncmp(kLine, "memory map ", 11)
                && strncmp(kLine, "memory map:", 11))
            continue;

        while (fgets(kLine, sizeof(kLine), mFp)) {
            if (!strcmp(kLine, "\n"))
                return true;

            if (!strncmp(kLine, "--- --- --- ---", 15))
                return true;

            if (!strncmp(kLine, "---------", 9))
                return true;

            if (!strncmp(kLine, "--->Fault address", 17))
                continue;

            int m;
            uint64_t high;
            uint64_t low;
            uint64_t memsz;
            Opencore::VirtualMemoryArea vma;
            char filename[256] = {'\0'};

            if (!strncmp(kLine, "--->", 4)) {
                if (!sscanf(kLine, "--->%" PRIx64 "\'%" PRIx64 "-%*17c %c%c%c %x %" PRIx64 " %255[^\n] %n",
                            &high, &low,
                            &vma.flags[0], &vma.flags[1], &vma.flags[2],
                            &vma.offset, &memsz, filename, &m))
                    continue;
            } else {
                if (!sscanf(kLine, "%" PRIx64 "\'%" PRIx64 "-%*17c %c%c%c %x %" PRIx64 " %255[^\n] %n",
                            &high, &low,
                            &vma.flags[0], &vma.flags[1], &vma.flags[2],
                            &vma.offset, &memsz, filename, &m))
                    continue;
            }

            vma.begin = (high << 32 | low);
            vma.end = vma.begin + memsz;
            vma.file = filename;

            std::size_t l_pos = vma.file.find(" (BuildId: ");
            if (l_pos != std::string::npos) {
                vma.file = vma.file.substr(0, l_pos);
                std::string buildid_str = filename;
                std::size_t r_pos = buildid_str.rfind(')');
                if (r_pos != std::string::npos) {
                    buildid_str = buildid_str.substr(l_pos + 11, 32);
                    vma.buildid = buildid_str;
                }
            }

            mMaps.push_back(vma);
        }
    }
    return true;
}

} // namespace x86_64
