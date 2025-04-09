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
#include "command/fake/core/arm/tombstone_parser.h"
#include <stdio.h>
#include <string.h>

namespace arm {

static char kLine[1024];

bool TombstoneParser::parse() {
    mCrashTid = 0;
    memset(&regs, 0x0, sizeof(regs));

    bool ret = parseCmdline() &&
               parseTid() &&
               parseRegister() &&
               parseBacktrace() &&
               parseMemory() &&
               parseMaps();

    LOGI("Cmdline: %s %s\n", mExecutable.c_str(), IsForce()? "(force)":"");
    LOGI("Tid: %d\n", mCrashTid);
    LOGI("r0  0x%08x  r1  0x%08x  r2  0x%08x  r3  0x%08x  \n", regs.regs[0], regs.regs[1], regs.regs[2], regs.regs[3]);
    LOGI("r4  0x%08x  r5  0x%08x  r6  0x%08x  r7  0x%08x  \n", regs.regs[4], regs.regs[5], regs.regs[6], regs.regs[7]);
    LOGI("r8  0x%08x  r9  0x%08x  r10 0x%08x  r11 0x%08x  \n", regs.regs[8], regs.regs[9], regs.regs[10], regs.regs[11]);
    LOGI("r12 0x%08x  sp  0x%08x  lr  0x%08x  pc  0x%08x  cpsr 0x%08x\n", regs.regs[12], regs.sp, regs.lr, regs.pc, regs.cpsr);

    for (const auto& lib : mLibs)
        LOGI("%s\n", lib.c_str());

    for (const auto& vma : mMaps)
        LOGD("[%" PRIx64 ", %" PRIx64 ") %s\n", vma.begin, vma.end, vma.file.c_str());

    return ret;
}

bool TombstoneParser::parseCmdline() {
    if (IsForce())
        return true;

    LOGD("%s ...\n", __func__);
    char value[256] = {'\0'};
    while (fgets(kLine, sizeof(kLine), mFp)) {
        if (sscanf(kLine, "Cmdline: %s", value)) {
            mExecutable = value;
            return true;
        }

        int pid, tid;
        if (sscanf(kLine, "pid: %d, tid: %d", &pid, &tid)) {
            rewind(mFp);
            return true;
        }
    }
    return false;
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
        if (!sscanf(kLine, " r0 %x r1 %x r2 %x r3 %x",
                    &regs.regs[0], &regs.regs[1], &regs.regs[2], &regs.regs[3]))
            continue;

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " r4 %x r5 %x r6 %x r7 %x", &regs.regs[4], &regs.regs[5], &regs.regs[6], &regs.regs[7]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " r8 %x r9 %x sl %x fp %x", &regs.regs[8], &regs.regs[9], &regs.regs[10], &regs.regs[11]);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " ip %x sp %x lr %x pc %x cpsr %x", &regs.regs[12], &regs.sp, &regs.lr, &regs.pc, &regs.cpsr);

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

            sscanf(kLine, "      #%*d pc %*x  %s %1023[^\n] %n", filename, buildid, &m);

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

        uint32_t addr;
        uint32_t value1;
        uint32_t value2;
        uint32_t value3;
        uint32_t value4;
        while (fgets(kLine, sizeof(kLine), mFp)) {
            if (!strcmp(kLine, "\n"))
                break;

            if (sscanf(kLine, "    %x %x %x %x %x", &addr, &value1, &value2, &value3, &value4)) {
                uint64_t tmp1 = (((uint64_t)value2) << 32) | value1;
                uint64_t tmp2 = (((uint64_t)value4) << 32) | value3;
                mMemorys.insert({addr, tmp1});
                mMemorys.insert({addr + 8, tmp2});
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
            uint64_t memsz;
            Opencore::VirtualMemoryArea vma;
            char filename[256] = {'\0'};

            if (!strncmp(kLine, "--->", 4)) {
                if (!sscanf(kLine, "--->%" PRIx64 "-%*x %c%c%c %x %" PRIx64 " %255[^\n] %n",
                            &vma.begin,
                            &vma.flags[0], &vma.flags[1], &vma.flags[2],
                            &vma.offset, &memsz, filename, &m))
                    continue;
            } else {
                if (!sscanf(kLine, "%" PRIx64 "-%*x %c%c%c %x %" PRIx64 " %255[^\n] %n",
                            &vma.begin,
                            &vma.flags[0], &vma.flags[1], &vma.flags[2],
                            &vma.offset, &memsz, filename, &m))
                    continue;
            }

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

            if (!IsForce()) {
                l_pos = vma.file.find("app_process");
                if (l_pos != std::string::npos)
                    mExecutable = vma.file;
            }

            if (vma.file == mExecutable) {
                std::string libname_append_buildid = vma.file;
                libname_append_buildid.append(":");
                libname_append_buildid.append(vma.buildid);
                mLibs.insert(libname_append_buildid);
            }
        }
    }
    return true;
}

} // namespace arm
