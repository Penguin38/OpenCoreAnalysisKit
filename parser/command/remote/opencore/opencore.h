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
#define I386_MACHINE    "i386"
#define I686_MACHINE    "i686"
#define X86_MACHINE     "x86"
#define ARM64_MACHINE   "arm64"
#define AARCH64_MACHINE "aarch64"
#define ARM_MACHINE     "arm"
#define V7L_MACHINE     "v7l"
#define V8L_MACHINE     "v8l"
#define ARMV7L_MACHINE  "armv7l"
#define ARMV8L_MACHINE  "armv8l"

#define ELFCOREMAGIC "CORE"
#define NOTE_CORE_NAME_SZ 5
#define ELFLINUXMAGIC "LINUX"
#define NOTE_LINUX_NAME_SZ 6

class Opencore {
public:
    static constexpr int FLAG_CORE = 1 << 0;
    static constexpr int FLAG_PROCESS_COMM = 1 << 1;
    static constexpr int FLAG_PID = 1 << 2;
    static constexpr int FLAG_THREAD_COMM = 1 << 3;
    static constexpr int FLAG_TID = 1 << 4;
    static constexpr int FLAG_TIMESTAMP = 1 << 5;
    static constexpr int FLAG_ALL = FLAG_CORE | FLAG_PROCESS_COMM | FLAG_PID
                              | FLAG_THREAD_COMM | FLAG_TID | FLAG_TIMESTAMP;

    static constexpr int INVALID_TID = 0;

    static constexpr int FILTER_NONE = 0x0;
    static constexpr int FILTER_SPECIAL_VMA = 1 << 0;
    static constexpr int FILTER_FILE_VMA = 1 << 1;
    static constexpr int FILTER_SHARED_VMA = 1 << 2;
    static constexpr int FILTER_SANITIZER_SHADOW_VMA = 1 << 3;
    static constexpr int FILTER_NON_READ_VMA = 1 << 4;
    static constexpr int FILTER_SIGNAL_CONTEXT = 1 << 5;
    static constexpr int FILTER_MINIDUMP = 1 << 6;

    static constexpr int VMA_NORMAL = 0;
    static constexpr int VMA_NULL = 1 << 0;
    static constexpr int VMA_INCLUDE = 1 << 1;

    static int Dump(int argc, char* const argv[]);
    static void Usage();

    Opencore() {
        flag = FLAG_CORE
             | FLAG_PID
             | FLAG_PROCESS_COMM
             | FLAG_TIMESTAMP;
        pid = INVALID_TID;
        tid = INVALID_TID;
        filter = FILTER_NONE;
        extra_note_filesz = 0;
        page_size = sysconf(_SC_PAGE_SIZE);
        align_size = ELF_PAGE_SIZE;
        zero = nullptr;
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

    struct ThreadRecord {
        int pid;
        bool attached;
    };

    void setDir(const char* d) { dir = d; }
    void setPid(int p) { pid = p; }
    void setTid(int t) { tid = t; }
    void setFilter(int f) { filter = f; }
    std::string& getDir() { return dir; }
    int getFlag() { return flag; }
    int getPid() { return pid; }
    int getTid() { return tid; }
    int getFilter() { return filter; }
    int getExtraNoteFilesz() { return extra_note_filesz; }
    bool Coredump(const char* filename);
    virtual ~Opencore();
    virtual bool DoCoredump(const char* filename) { return false; }
    virtual int NeedFilterFile(Opencore::VirtualMemoryArea& vma) { return VMA_NORMAL; }
    virtual int getMachine() { return EM_NONE; }
    int IsFilterSegment(Opencore::VirtualMemoryArea& vma);
    void StopTheWorld(int pid);
    bool StopTheThread(int tid);
    void Continue();
    static bool IsBit64(int pid);
    static std::string DecodeMachine(int pid);
    static std::unique_ptr<Opencore> MakeArch(std::string& type);
    static void ParseMaps(int pid, std::vector<VirtualMemoryArea>& maps);
    static void TermStopHandle(int signal);
    static bool isAlive(int pid);
protected:
    int extra_note_filesz;
    std::vector<ThreadRecord> threads;
    std::vector<VirtualMemoryArea> maps;
    uint8_t* zero;
    uint32_t align_size;
    uint32_t page_size;
private:
    std::string dir;
    int flag;
    int pid;
    int tid;
    int filter;
};

#endif // PARSER_COMMAND_REMOTE_OPENCORE_OPENCORE_H_
