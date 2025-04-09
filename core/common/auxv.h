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

#ifndef CORE_COMMON_AUXV_H_
#define CORE_COMMON_AUXV_H_

#include <stdint.h>
#include <sys/types.h>
#include <iostream>

#define AT_NULL                  0           /* end of vector */
#define AT_IGNORE                1           /* entry should be ignored */
#define AT_EXECFD                2           /* file descriptor of program */
#define AT_PHDR                  3           /* program headers for program */
#define AT_PHENT                 4           /* size of program header entry */
#define AT_PHNUM                 5           /* number of program headers */
#define AT_PAGESZ                6           /* system page size */
#define AT_BASE                  7           /* base address of interpreter */
#define AT_FLAGS                 8           /* flags */
#define AT_ENTRY                 9           /* entry point of program */
#define AT_NOTELF                10          /* program is not ELF */
#define AT_UID                   11          /* real uid */
#define AT_EUID                  12          /* effective uid */
#define AT_GID                   13          /* real gid */
#define AT_EGID                  14          /* effective gid */
#define AT_PLATFORM              15          /* string identifying CPU for optimizations */
#define AT_HWCAP                 16          /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK                17          /* frequency at which times() increments */
/* AT_* values 18 through 22      are reserved */
#define AT_SECURE                23          /* secure mode boolean */
#define AT_BASE_PLATFORM         24          /* string identifying real platform, may
                                              * differ from AT_PLATFORM. */
#define AT_RANDOM                25          /* address of 16 random bytes */
#define AT_HWCAP2                26          /* extension of AT_HWCAP */
                                 
#define AT_EXECFN                31          /* filename of program */
#define AT_SYSINFO               32
#define AT_SYSINFO_EHDR          33
                                 
#define AT_L1I_CACHESHAPE        34
#define AT_L1D_CACHESHAPE        35
#define AT_L2_CACHESHAPE         36
#define AT_L3_CACHESHAPE         37
                                 
#define AT_L1I_CACHESIZE         40
#define AT_L1I_CACHEGEOMETRY     41
#define AT_L1D_CACHESIZE         42
#define AT_L1D_CACHEGEOMETRY     43
#define AT_L2_CACHESIZE          44
#define AT_L2_CACHEGEOMETRY      45
#define AT_L3_CACHESIZE          46
#define AT_L3_CACHEGEOMETRY      47

#define AT_MINSIGSTKSZ           51

class NoteBlock;

class Auxv {
public:
    inline uint64_t type() { return mType; }
    inline uint64_t value() { return mValue; }

    inline std::string to_string() {
        switch(mType) {
            case AT_NULL: return "AT_NULL";
            case AT_IGNORE: return "AT_IGNORE";
            case AT_EXECFD: return "AT_EXECFD";
            case AT_PHDR: return "AT_PHDR";
            case AT_PHENT: return "AT_PHENT";
            case AT_PHNUM: return "AT_PHNUM";
            case AT_PAGESZ: return "AT_PAGESZ";
            case AT_BASE: return "AT_BASE";
            case AT_FLAGS: return "AT_FLAGS";
            case AT_ENTRY: return "AT_ENTRY";
            case AT_NOTELF: return "AT_NOTELF";
            case AT_UID: return "AT_UID";
            case AT_EUID: return "AT_EUID";
            case AT_GID: return "AT_GID";
            case AT_EGID: return "AT_EGID";
            case AT_PLATFORM: return "AT_PLATFORM";
            case AT_HWCAP: return "AT_HWCAP";
            case AT_CLKTCK: return "AT_CLKTCK";
            case AT_SECURE: return "AT_SECURE";
            case AT_BASE_PLATFORM: return "AT_BASE_PLATFORM";
            case AT_RANDOM: return "AT_RANDOM";
            case AT_HWCAP2: return "AT_HWCAP2";
            case AT_EXECFN: return "AT_EXECFN";
            case AT_SYSINFO: return "AT_SYSINFO";
            case AT_SYSINFO_EHDR: return "AT_SYSINFO_EHDR";
            case AT_MINSIGSTKSZ: return "AT_MINSIGSTKSZ";
            default:
                return "AT_UNKNOW";
        }
    }

    Auxv(uint64_t t, uint64_t v) : mType(t), mValue(v) {}
    void bind(NoteBlock* block, uint64_t addr);
    void setValue(uint64_t value);
    void setType(uint64_t type);
    void setTypeAndValue(uint64_t type, uint64_t value);
    uint64_t address() { return mAuxv; }
    ~Auxv() {}
private:
    //  auxv member
    uint64_t mType;
    uint64_t mValue;
    NoteBlock* mBlock;
    uint64_t mAuxv;
};

#endif  // CORE_COMMON_AUXV_H_
