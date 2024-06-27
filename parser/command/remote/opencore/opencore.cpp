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
#include "base/utils.h"
#include "command/env.h"
#include "command/remote/opencore/opencore.h"
#include "command/remote/opencore/x86_64/opencore.h"
#include "command/remote/opencore/x86/opencore.h"
#include "command/remote/opencore/arm64/opencore.h"
#include "command/remote/opencore/arm/opencore.h"
#include "command/remote/opencore/riscv64/opencore.h"
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <memory>

int Opencore::Dump(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {"filter",  required_argument, 0, 'f'},
        {"dir",     required_argument, 0, 'd'},
        {"output",  required_argument, 0, 'o'},
        {"machine", required_argument, 0, 'm'},
    };

    int pid = 0;
    int filter = FILTER_SPECIAL_VMA
               | FILTER_SANITIZER_SHADOW_VMA
               | FILTER_NON_READ_VMA;
    char* dir = const_cast<char *>(Env::CurrentDir());
    char* filename = nullptr;
    char* machine = const_cast<char *>(NONE_MACHINE);
    while ((opt = getopt_long(argc, argv, "p:f:d:o:m:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                pid = std::atoi(optarg);
                break;
            case 'f':
                filter = Utils::atol(optarg);
                break;
            case 'd':
                dir = optarg;
                break;
            case 'o':
                filename = optarg;
                break;
            case 'm':
                machine = optarg;
                break;
        }
    }

    std::unique_ptr<Opencore> impl;
    std::string type = machine ? machine : "";
    if (type == "arm64" || type == "ARM64") {
        impl = std::make_unique<arm64::Opencore>();
    } else if (type == "arm" || type == "ARM") {
        impl = std::make_unique<arm::Opencore>();
    } else if (type == "x86_64" || type == "X86_64") {
        impl = std::make_unique<x86_64::Opencore>();
    } else if (type == "x86" || type == "X86") {
        impl = std::make_unique<x86::Opencore>();
    } else if (type == "riscv64" || type == "RISCV64") {
        impl = std::make_unique<riscv64::Opencore>();
    }

    if (impl) {
        impl->setDir(dir);
        impl->setPid(pid);
        impl->setFilter(filter);
        impl->Coredump(filename);
    }
    return 0;
}

bool Opencore::Coredump(const char* filename) {
    int ori_dumpable;
    int flag;
    int pid = 0;
    char comm[16];
    bool need_split = false;
    bool need_restore_dumpable = false;
    bool need_restore_ptrace = false;
    std::string output;

    if (getDir().length() > 0) {
        output.append(getDir()).append("/");
    }
    if (!filename) {
        flag = getFlag();
        if (!(flag & FLAG_ALL)) {
            flag |= FLAG_CORE;
            flag |= FLAG_TID;
        }

        if (flag & FLAG_CORE)
            output.append("core.");

        if (flag & FLAG_PROCESS_COMM) {
            pid = getPid();
            char comm_path[32];
            snprintf(comm_path, sizeof(comm_path), "/proc/%d/comm", pid);
            int fd = open(comm_path, O_RDONLY);
            if (fd > 0) {
                memset(&comm, 0x0, sizeof(comm));
                int rc = read(fd, &comm, sizeof(comm) - 1);
                if (rc > 0) {
                    for (int i = 0; i < rc; i++) {
                        if (comm[i] == '\n') {
                            comm[i] = 0;
                            break;
                        }
                    }
                    comm[rc] = 0;
                    output.append(comm);
                } else {
                    output.append("unknown");
                }
                close(fd);
            }
            need_split = true;
        }

        if (flag & FLAG_PID) {
            if (!pid)
                pid = getPid();
            if (need_split)
                output.append("_");
            output.append(std::to_string(pid));
            need_split = true;
        }

        if (flag & FLAG_TIMESTAMP) {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            if (need_split)
                output.append("_");
            output.append(std::to_string(tv.tv_sec));
        }
    } else {
        output.append(filename);
    }

    ori_dumpable = prctl(PR_GET_DUMPABLE);
    if (!prctl(PR_SET_DUMPABLE, 1))
        need_restore_dumpable = true;

    if (!prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY))
        need_restore_ptrace = true;

    bool ret = DoCoredump(output.c_str());

    if (need_restore_dumpable) prctl(PR_SET_DUMPABLE, ori_dumpable);
    if (need_restore_ptrace) prctl(PR_SET_PTRACER, 0);

    return ret;
}

Opencore::~Opencore() {
}

bool Opencore::IsFilterSegment(char* flags, int inode, std::string segment, int offset) {
    int filter = getFilter();
    if (filter & FILTER_SPECIAL_VMA) {
        if (segment == "/dev/binderfs/hwbinder"
                || segment == "/dev/binderfs/binder"
                || segment == "[vvar]"
                || segment == "/dev/mali0"
           ) {
            return true;
        }
    }

    if (filter & FILTER_FILE_VMA) {
        if (inode > 0 && flags[1] == '-')
            return NeedFilterFile(segment.c_str(), offset);
    }

    if (filter & FILTER_SHARED_VMA) {
        if (flags[3] == 's' || flags[3] == 'S')
            return true;
    }

    if (filter & FILTER_SANITIZER_SHADOW_VMA) {
        if (segment == "[anon:low shadow]"
                || segment == "[anon:high shadow]"
                || (segment.compare(0, 12, "[anon:hwasan") == 0))
            return true;
    }

    if (filter & FILTER_NON_READ_VMA) {
        if (flags[0] == '-')
            return true;
    }
    return false;
}

void Opencore::StopTheWorld(int pid) {
    char task_dir[32];
    struct dirent *entry;
    snprintf(task_dir, sizeof(task_dir), "/proc/%d/task", pid);
    DIR *dp = opendir(task_dir);
    if (dp) {
        while ((entry=readdir(dp)) != NULL) {
            if (!strncmp(entry->d_name, ".", 1)) {
                continue;
            }

            pid_t tid = atoi(entry->d_name);
            if (ptrace(PTRACE_ATTACH, tid, NULL, 0) < 0) {
                LOGI("%s %d: %s\n", __func__ , tid, strerror(errno));
                continue;
            }
            pids.push_back(tid);
            int status = 0;
            waitpid(tid, &status, WUNTRACED);
        }
        closedir(dp);
    }
}

void Opencore::StopTheThread(int tid) {
    if (ptrace(PTRACE_ATTACH, tid, NULL, 0) < 0) {
        LOGI("%s %d: %s\n", __func__ , tid, strerror(errno));
        return;
    }
    pids.push_back(tid);
    int status = 0;
    waitpid(tid, &status, WUNTRACED);
}

void Opencore::Usage() {
    LOGI("Usage: remote core -p <PID> -m <MACHINE> [Option]...\n");
    LOGI("Option:\n");
    LOGI("   --pid|-p <PID>\n");
    LOGI("   --dir|-d <DIR>\n");
    LOGI("   --machine|-m <Machine>\n");
    LOGI("Machine:\n");
    LOGI("     { arm64, arm, x86_64, x86, riscv64 }\n");
    LOGI("   --output|-o <COREFILE>\n");
    LOGI("   --filter|-f <Filter>\n");
    LOGI("Filter: (0x19 default)\n");
    LOGI("     0x01: filter-special-vma\n");
    LOGI("     0x02: filter-file-vma\n");
    LOGI("     0x04: filter-shared-vma\n");
    LOGI("     0x08: filter-sanitizer-shadow-vma\n");
    LOGI("     0x10: filter-non-read-vma\n");
}
