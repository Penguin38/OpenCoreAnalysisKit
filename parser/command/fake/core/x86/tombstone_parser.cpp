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
#include "command/fake/core/x86/tombstone_parser.h"
#include <stdio.h>
#include <string.h>

namespace x86 {

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
    LOGI("eax 0x%08x  ebx 0x%08x  ecx 0x%08x  edx 0x%08x  \n", regs.eax, regs.ebx, regs.ecx, regs.edx);
    LOGI("edi 0x%08x  esi 0x%08x  \n", regs.edi, regs.esi);
    LOGI("ebp 0x%08x  esp 0x%08x  eip 0x%08x  \n", regs.ebp, regs.esp, regs.eip);

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
        if (!sscanf(kLine, " eax %x ebx %x ecx %x edx %x",
                    &regs.eax, &regs.ebx, &regs.ecx, &regs.edx))
            continue;

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " edi %x esi %x", &regs.edi, &regs.esi);

        fgets(kLine, sizeof(kLine), mFp);
        sscanf(kLine, " ebp %x esp %x eip %x", &regs.ebp, &regs.esp, &regs.eip);

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

} // namespace x86
