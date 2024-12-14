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

#ifndef PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_
#define PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_

#include "common/elf.h"
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

#define NONE_MACHINE    "NONE"
#define X86_64_MACHINE  "x86_64"
#define X86_MACHINE     "x86"
#define ARM64_MACHINE   "arm64"
#define AARCH64_MACHINE "aarch64"
#define ARM_MACHINE     "arm"

#define ELFCOREMAGIC "CORE"
#define NOTE_CORE_NAME_SZ 5
#define ELFLINUXMAGIC "LINUX"
#define NOTE_LINUX_NAME_SZ 6

#define GENMASK_UL(h, l) (((~0UL) << (l)) & (~0UL >> (64 - 1 - (h))))

class Opencore {
public:
    static const int FLAG_CORE = 1 << 0;
    static const int FLAG_PROCESS_COMM = 1 << 1;
    static const int FLAG_PID = 1 << 2;
    static const int FLAG_THREAD_COMM = 1 << 3;
    static const int FLAG_TID = 1 << 4;
    static const int FLAG_TIMESTAMP = 1 << 5;
    static const int FLAG_ALL = FLAG_CORE | FLAG_PROCESS_COMM | FLAG_PID
                              | FLAG_THREAD_COMM | FLAG_TID | FLAG_TIMESTAMP;

    static const int INVALID_TID = 0;

    static const int FILTER_NONE = 0x0;
    static const int FILTER_SPECIAL_VMA = 1 << 0;
    static const int FILTER_FILE_VMA = 1 << 1;
    static const int FILTER_SHARED_VMA = 1 << 2;
    static const int FILTER_SANITIZER_SHADOW_VMA = 1 << 3;
    static const int FILTER_NON_READ_VMA = 1 << 4;

    static int Dump(int argc, char* const argv[]);
    static void Usage();

    Opencore() {
        flag = FLAG_CORE
             | FLAG_PID
             | FLAG_PROCESS_COMM
             | FLAG_TIMESTAMP;
        pid = INVALID_TID;
        filter = FILTER_NONE;
        extra_note_filesz = 0;
        page_size = sysconf(_SC_PAGE_SIZE);
        align_size = ELF_PAGE_SIZE;
        zero = (uint8_t*)malloc(align_size);
    }

    struct VirtualMemoryArea {
        uint64_t begin;
        uint64_t end;
        char     flags[4];
        uint32_t offset;
        uint32_t major;
        uint32_t minor;
        uint64_t inode;
        std::string file;
        std::string buildid;
    };

    void setDir(const char* d) { dir = d; }
    void setPid(int p) { pid = p; }
    void setFilter(int f) { filter = f; }
    std::string& getDir() { return dir; }
    int getFlag() { return flag; }
    int getPid() { return pid; }
    int getFilter() { return filter; }
    int getExtraNoteFilesz() { return extra_note_filesz; }
    bool Coredump(const char* filename);
    virtual ~Opencore();
    virtual bool DoCoredump(const char* filename) { return false; }
    virtual bool NeedFilterFile(Opencore::VirtualMemoryArea& vma) { return false; }
    virtual int getMachine() { return EM_NONE; }
    bool IsFilterSegment(Opencore::VirtualMemoryArea& vma);
    void StopTheWorld(int pid);
    void StopTheThread(int tid);
    static bool IsBit64(int pid);
    static std::string DecodeMachine(int pid);
    static std::unique_ptr<Opencore> MakeArch(std::string& type);
    static void ParseMaps(int pid, std::vector<VirtualMemoryArea>& maps);
protected:
    int extra_note_filesz;
    std::vector<int> pids;
    std::vector<VirtualMemoryArea> maps;
    uint8_t* zero;
    uint64_t align_size;
    uint64_t page_size;
private:
    std::string dir;
    int flag;
    int pid;
    int filter;
};

#endif // PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_
