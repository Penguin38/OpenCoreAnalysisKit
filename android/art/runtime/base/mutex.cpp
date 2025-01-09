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

#include "api/core.h"
#include "android.h"
#include "arm64/inst/constant.h"
#include "arm/inst/constant.h"
#include "common/elf.h"
#include "runtime/base/mutex.h"

struct BaseMutex_OffsetTable __BaseMutex_offset__;
struct Mutex_OffsetTable __Mutex_offset__;
struct ReaderWriterMutex_OffsetTable __ReaderWriterMutex_offset__;

namespace art {

void BaseMutex::Init() {
    Android::RegisterSdkListener(Android::M, art::BaseMutex::Init23);
    Android::RegisterSdkListener(Android::Q, art::BaseMutex::Init29);

    Android::RegisterSdkListener(Android::M, art::Mutex::Init23);
    Android::RegisterSdkListener(Android::O, art::Mutex::Init26);

    Android::RegisterSdkListener(Android::M, art::ReaderWriterMutex::Init23);
    Android::RegisterSdkListener(Android::O, art::ReaderWriterMutex::Init26);
}

void BaseMutex::Init23() {
    if (CoreApi::Bits() == 64) {
        __BaseMutex_offset__ = {
            .vtbl = 0,
            .vtbl_IsMutex = 0,
            .vtbl_IsReaderWriterMutex = 8,
            .vtbl_IsMutatorMutex = 16,
            .name_ = 16,
        };
    } else {
        __BaseMutex_offset__ = {
            .vtbl = 0,
            .vtbl_IsMutex = 0,
            .vtbl_IsReaderWriterMutex = 4,
            .vtbl_IsMutatorMutex = 8,
            .name_ = 8,
        };
    }
}

void BaseMutex::Init29() {
    if (CoreApi::Bits() == 64) {
        __BaseMutex_offset__ = {
            .vtbl = 0,
            .vtbl_IsMutex = 0,
            .vtbl_IsReaderWriterMutex = 8,
            .vtbl_IsMutatorMutex = 16,
            .name_ = 8,
        };
    } else {
        __BaseMutex_offset__ = {
            .vtbl = 0,
            .vtbl_IsMutex = 0,
            .vtbl_IsReaderWriterMutex = 4,
            .vtbl_IsMutatorMutex = 8,
            .name_ = 4,
        };
    }
}

void Mutex::Init23() {
    if (CoreApi::Bits() == 64) {
        __Mutex_offset__ = {
            .exclusive_owner_ = 32,
        };
    } else {
        __Mutex_offset__ = {
            .exclusive_owner_ = 24,
        };
    }
}

void Mutex::Init26() {
    if (CoreApi::Bits() == 64) {
        __Mutex_offset__ = {
            .exclusive_owner_ = 24,
        };
    } else {
        __Mutex_offset__ = {
            .exclusive_owner_ = 16,
        };
    }
}

void ReaderWriterMutex::Init23() {
    if (CoreApi::Bits() == 64) {
        __ReaderWriterMutex_offset__ = {
            .state_ = 24,
            .exclusive_owner_ = 32,
        };
    } else {
        __ReaderWriterMutex_offset__ = {
            .state_ = 16,
            .exclusive_owner_ = 24,
        };
    }
}

void ReaderWriterMutex::Init26() {
    if (CoreApi::Bits() == 64) {
        __ReaderWriterMutex_offset__ = {
            .state_ = 20,
            .exclusive_owner_ = 24,
        };
    } else {
        __ReaderWriterMutex_offset__ = {
            .state_ = 12,
            .exclusive_owner_ = 16,
        };
    }
}

static BaseMutex::LockTable kLockTable[] = {
    {"user code suspension lock",                  "Mutex"},
    {"instrument entrypoint lock",                 "Mutex"},
    {"mutator lock",                        "MutatorMutex"},
    {"heap bitmap lock",               "ReaderWriterMutex"},
    {"trace lock",                                 "Mutex"},
    {"runtime shutdown lock",                      "Mutex"},
    {"runtime thread pool lock",                   "Mutex"},
    {"profiler lock",                              "Mutex"},
    {"Deoptimization lock",                        "Mutex"},
    {"AllocTracker lock",                          "Mutex"},
    {"thread list lock",                           "Mutex"},
    {"breakpoint lock",                "ReaderWriterMutex"},
    {"SubtypeCheck lock",                          "Mutex"},
    {"ClassLinker classes lock",       "ReaderWriterMutex"},
    {"allocated monitor ids lock",                 "Mutex"},
    {"allocated thread ids lock",                  "Mutex"},
    {"modify_ldt lock",                            "Mutex"},
    {"ClassLinker dex lock",           "ReaderWriterMutex"},
    {"DexCache lock",                              "Mutex"},
    {"JNI shared libraries map lock",              "Mutex"},
    {"OatFile manager lock",           "ReaderWriterMutex"},
    {"verifier deps lock",             "ReaderWriterMutex"},
    {"host dlopen handles lock",                   "Mutex"},
    {"InternTable lock",                           "Mutex"},
    {"ReferenceProcessor lock",                    "Mutex"},
    {"ReferenceQueue cleared references lock",     "Mutex"},
    {"ReferenceQueue weak references lock",        "Mutex"},
    {"ReferenceQueue finalizer references lock",   "Mutex"},
    {"ReferenceQueue phantom references lock",     "Mutex"},
    {"ReferenceQueue soft references lock",        "Mutex"},
    {"JNI global reference table lock","ReaderWriterMutex"},
    {"JNI weak global reference table lock",       "Mutex"},
    {"JNI function table lock",                    "Mutex"},
    {"Thread::custom_tls_ lock",                   "Mutex"},
    {"Jit code cache",                             "Mutex"},
    {"CHA lock",                                   "Mutex"},
    {"Native debug interface lock",                "Mutex"},
    {"JNI id map lock",                "ReaderWriterMutex"},
    {"abort lock",                                 "Mutex"},
    {"thread suspend count lock",                  "Mutex"},
    {"unexpected signal lock",                     "Mutex"},
    {"logging lock",                               "Mutex"},
    {"a monitor lock",                             "Mutex"},
};

api::MemoryRef& BaseMutex::GetVTBL() {
    if (!vtbl_cache.Ptr()) {
        vtbl_cache = vtbl();
        vtbl_cache.Prepare(false);
    }
    return vtbl_cache;
}

const char* BaseMutex::GetName() {
    api::MemoryRef name_ = name();
    return reinterpret_cast<const char*>(name_.Real());
}


/*
 *          ---------------------
 * vtbl -> |        IsMutex      |----
 *          ---------------------    |
 *         | IsReaderWriterMutex |   |
 *          ---------------------    --> machine inst
 *         |    IsMutatorMutex   |
 *          ---------------------
 */
bool BaseMutex::IsSpecialMutex(const char* type, uint32_t off) {
    api::MemoryRef cache = GetVTBL();
    if (!cache.IsValid())
        return false;

    cache = cache.valueOf(off);
    if (!cache.IsValid())
        return false;

    bool ret = false;
    int machine = CoreApi::GetMachine();
    switch(machine) {
        case EM_AARCH64: {
            // mov w0 type
            // ret
            uint32_t* instptr = reinterpret_cast<uint32_t *>(cache.Real());
            /* 0xaa1f03e0 --> mov x0, xzr
             * 0x2a1f03e0 --> mov w0, wzr
             *
             * mov rd, #<imm>
             * |31|30 29|28 27 26 25 24 23|22 21| 20           5|4          0|
             * |sf| 0  0| 1  0  0  1  0  1|  hw | --- imm16 --- | --- Rd --- |
             *      opc
             *
             * 0x320003e0 --> orr w0, wzr, #1
             */
            int retry = 2;
            do {
                uint32_t inst = instptr[0];
                if (inst == 0xaa1f03e0 || inst == 0x2a1f03e0) {
                    break;
                }

                if (inst == 0x320003e0) {
                    ret = true;
                    break;
                }

                uint32_t checkop = inst & arm64::MoveWideImmediateOp::MoveWideImmediateMask;
                if (checkop != arm64::MoveWideImmediateOp::MOVZ_w
                        && checkop != arm64::MoveWideImmediateOp::MOVZ_x) {
                    retry--;
                    instptr++;
                    continue;
                }

                uint32_t rd = (inst & 0x1F);
                uint32_t opc = (inst >> 29) & 0b11;

                if (rd != 0x0 || opc != 0b10) {
                    retry--;
                    instptr++;
                    continue;
                }

                uint32_t hw = ((inst >> 21) & 0x3);
                uint32_t imm16 = ((inst >> 5) & 0xFFFF);
                uint32_t pos = hw << 4;
                uint32_t imm = imm16 << pos;
                if (imm > 1) {
                    retry--;
                    instptr++;
                    continue;
                }

                ret = (imm == 1);
                break;
            } while (retry > 0);
        } break;
        case EM_ARM: {
            // thumb inst
            if (cache.Ptr() & 0x1) {
                // movs r0, type
                cache.MovePtr(-1);

                /*
                 * mov rd, #expr
                 * |15          11|10   8| 7            0|
                 * | 0| 0| 1| 0| 0|  rd  | -- immed_8 -- |
                 */

                uint16_t inst = *reinterpret_cast<uint16_t *>(cache.Real());
                uint32_t checkop = inst >> 11;
                uint32_t rd = (inst >> 8) & 0x7;
                uint32_t imm8 = inst & 0xFF;
                if (checkop != thumb::mov_expr || rd != 0x0
                        || imm8 > 1)
                    break;

                ret = imm8 == 1;
            }
        } break;
        case EM_RISCV:
            break;
        case EM_X86_64: {
            /*
             * mov $type,%rax  // xor %rax,%rax
             * retq
             */
            uint64_t inst = *reinterpret_cast<uint64_t *>(cache.Real());
            // xor
            if ((inst & 0xFFFF) == 0xc031) {
            } else {
                inst = inst & 0xFFFFFFFFFFULL;
                if ((inst & 0xb8) == 0xb8)
                    ret = ((inst >> 8) & 0xFF) == 1;
            }
        } break;
        case EM_386: {
            uint64_t inst = *reinterpret_cast<uint64_t *>(cache.Real());
            if ((inst & 0xFFFF) == 0xc031) {
            } else {
                inst = inst & 0xFFFFFFFFFFULL;
                if ((inst & 0xb8) == 0xb8)
                    ret = ((inst >> 8) & 0xFF) == 1;
            }

            if (ret)
                break;
            /*
             * <0>  push %ebp
             * <1>  mov %esp,%ebp
             * <3>  and $0xfffffffc,%esp
             * <6>  mov $type,%eax  // xor %eax,%eax
             * ...
             */
            inst = *reinterpret_cast<uint64_t *>(cache.Real() + 0x6);
            // xor
            if ((inst & 0xFFFF) == 0xc031) {
            } else {
                inst = inst & 0xFFFFFFFFFFULL;
                ret = ((inst >> 8) & 0xFF) == 1;
            }
        } break;
    }

    if (ret) return ret;

    // second check
    const char* name = GetName();
    for (int i = 0; i < sizeof(kLockTable)/sizeof(kLockTable[0]); i++) {
        if (!strcmp(name, kLockTable[i].name)) {
            return !strcmp(kLockTable[i].type, type);
        }
    }
    return false;
}

bool BaseMutex::IsMutex() {
    return IsSpecialMutex("Mutex", OFFSET(BaseMutex, vtbl_IsMutex));
}

bool BaseMutex::IsReaderWriterMutex() {
    return IsSpecialMutex("ReaderWriterMutex", OFFSET(BaseMutex, vtbl_IsReaderWriterMutex));
}

bool BaseMutex::IsMutatorMutex() {
    return IsSpecialMutex("MutatorMutex", OFFSET(BaseMutex, vtbl_IsMutatorMutex));
}

uint32_t Mutex::GetExclusiveOwnerTid() {
    return exclusive_owner();
}

uint32_t ReaderWriterMutex::GetExclusiveOwnerTid() {
    int32_t state_ = state();
    if (state_ == 0) {
        return 0;
    } else if (state_ > 0) {
        return -1;
    } else {
        return exclusive_owner();
    }
}

} // namespace art
