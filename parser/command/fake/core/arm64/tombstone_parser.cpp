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
#include "command/fake/core/arm64/tombstone_parser.h"
#include <stdio.h>
#include <string.h>

namespace arm64 {

static char kLine[1024];

bool TombstoneParser::parse() {
    tagged_addr_ctrl = -1;
    pac_enabled_keys = -1;
    mCrashTid = 0;
    memset(&regs, 0x0, sizeof(regs));

    bool ret = parsePauth() &&
               parseTid() &&
               parseRegister() &&
               parseBacktrace() &&
               parseMemory() &&
               parseMaps();

    LOGI("Tid: %d\n", mCrashTid);
    if (tagged_addr_ctrl) LOGI("tagged_addr_ctrl %" PRIx64 "\n", tagged_addr_ctrl);
    if (pac_enabled_keys) LOGI("pac_enabled_keys %" PRIx64 "\n", pac_enabled_keys);
    LOGI("x0  0x%016" PRIx64 "  x1  0x%016" PRIx64 "  x2  0x%016" PRIx64 "  x3  0x%016" PRIx64 "  \n", regs.regs[0], regs.regs[1], regs.regs[2], regs.regs[3]);
    LOGI("x4  0x%016" PRIx64 "  x5  0x%016" PRIx64 "  x6  0x%016" PRIx64 "  x7  0x%016" PRIx64 "  \n", regs.regs[4], regs.regs[5], regs.regs[6], regs.regs[7]);
    LOGI("x8  0x%016" PRIx64 "  x9  0x%016" PRIx64 "  x10 0x%016" PRIx64 "  x11 0x%016" PRIx64 "  \n", regs.regs[8], regs.regs[9], regs.regs[10], regs.regs[11]);
    LOGI("x12 0x%016" PRIx64 "  x13 0x%016" PRIx64 "  x14 0x%016" PRIx64 "  x15 0x%016" PRIx64 "  \n", regs.regs[12], regs.regs[13], regs.regs[14], regs.regs[15]);
    LOGI("x16 0x%016" PRIx64 "  x17 0x%016" PRIx64 "  x18 0x%016" PRIx64 "  x19 0x%016" PRIx64 "  \n", regs.regs[16], regs.regs[17], regs.regs[18], regs.regs[19]);
    LOGI("x20 0x%016" PRIx64 "  x21 0x%016" PRIx64 "  x22 0x%016" PRIx64 "  x23 0x%016" PRIx64 "  \n", regs.regs[20], regs.regs[21], regs.regs[22], regs.regs[23]);
    LOGI("x24 0x%016" PRIx64 "  x25 0x%016" PRIx64 "  x26 0x%016" PRIx64 "  x27 0x%016" PRIx64 "  \n", regs.regs[24], regs.regs[25], regs.regs[26], regs.regs[27]);
    LOGI("x28 0x%016" PRIx64 "  fp  0x%016" PRIx64 "  \n", regs.regs[28], regs.regs[29]);
    LOGI("lr  0x%016" PRIx64 "  sp  0x%016" PRIx64 "  pc  0x%016" PRIx64 "  pst 0x%016" PRIx64 "  \n", regs.regs[30], regs.sp, regs.pc, regs.pstate);

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

bool TombstoneParser::parsePauth() {
    LOGD("%s ...\n", __func__);
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (sscanf(kLine, "tagged_addr_ctrl: %" PRIx64 "", &tagged_addr_ctrl)) {
            if (fgets(kLine, sizeof(kLine), mFp))
                sscanf(kLine, "pac_enabled_keys: %" PRIx64 "", &pac_enabled_keys);
            break;
        }
    }
    rewind(mFp);
    return true;
}

bool TombstoneParser::parseRegister() {
    LOGD("%s ...\n", __func__);
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (!sscanf(kLine, " x0 %" PRIx64 " x1 %" PRIx64 " x2 %" PRIx64 " x3 %" PRIx64 "",
                    &regs.regs[0], &regs.regs[1], &regs.regs[2], &regs.regs[3]))
            continue;

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x4 %" PRIx64 " x5 %" PRIx64 " x6 %" PRIx64 " x7 %" PRIx64 "", &regs.regs[4], &regs.regs[5], &regs.regs[6], &regs.regs[7]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x8 %" PRIx64 " x9 %" PRIx64 " x10 %" PRIx64 " x11 %" PRIx64 "", &regs.regs[8], &regs.regs[9], &regs.regs[10], &regs.regs[11]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x12 %" PRIx64 " x13 %" PRIx64 " x14 %" PRIx64 " x15 %" PRIx64 "", &regs.regs[12], &regs.regs[13], &regs.regs[14], &regs.regs[15]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x16 %" PRIx64 " x17 %" PRIx64 " x18 %" PRIx64 " x19 %" PRIx64 "", &regs.regs[16], &regs.regs[17], &regs.regs[18], &regs.regs[19]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x20 %" PRIx64 " x21 %" PRIx64 " x22 %" PRIx64 " x23 %" PRIx64 "", &regs.regs[20], &regs.regs[21], &regs.regs[22], &regs.regs[23]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x24 %" PRIx64 " x25 %" PRIx64 " x26 %" PRIx64 " x27 %" PRIx64 "", &regs.regs[24], &regs.regs[25], &regs.regs[26], &regs.regs[27]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " x28 %" PRIx64 " x29 %" PRIx64 "", &regs.regs[28], &regs.regs[29]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " lr %" PRIx64 " sp %" PRIx64 " pc %" PRIx64 " pst %" PRIx64 "", &regs.regs[30], &regs.sp, &regs.pc, &regs.pstate);

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

} // namespace arm64
