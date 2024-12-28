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

#include "api/core.h"
#include "runtime/gc/space/space.h"
#include "runtime/runtime_globals.h"
#include "arm64/inst/constant.h"
#include "arm/inst/constant.h"
#include "common/elf.h"
#include "cxx/string.h"
#include "common/bit.h"

struct Space_OffsetTable __Space_offset__;
struct Space_SizeTable __Space_size__;
struct ContinuousSpace_OffsetTable __ContinuousSpace_offset__;
struct ContinuousSpace_SizeTable __ContinuousSpace_size__;

namespace art {
namespace gc {
namespace space {

void Space::Init() {
    if (CoreApi::Bits() == 64) {
        __Space_offset__ = {
            .vtbl = 0,
            .vtbl_GetType = 16,
            .name_ = 8,
        };
    } else {
        __Space_offset__ = {
            .vtbl = 0,
            .vtbl_GetType = 8,
            .name_ = 4,
        };
    }
}

void ContinuousSpace::Init() {
    if (CoreApi::Bits() == 64) {
        __ContinuousSpace_offset__ = {
            .begin_ = 40,
            .end_ = 48,
            .limit_ = 56,
        };
    } else {
        __ContinuousSpace_offset__ = {
            .begin_ = 20,
            .end_ = 24,
            .limit_ = 28,
        };
    }
}

const char* Space::GetName() {
    cxx::string name_(name(), this);
    return name_.c_str();
}


/*
 *          ---------
 * vtbl -> | ------ |
 *          --------
 *         | ------ |
 *          --------    --> machine inst
 *         | GetType| --|
 *          --------
 */
SpaceType Space::GetType() {
    if (type_cache != kSpaceTypeInvalidSpace)
        return type_cache;

    if (!vtbl_cache.Ptr()) {
        vtbl_cache = vtbl();
        vtbl_cache.Prepare(false);
    }

    api::MemoryRef getTypeCache(vtbl_cache.valueOf(OFFSET(Space, vtbl_GetType)), vtbl_cache);
    int machine = CoreApi::GetMachine();
    switch(machine) {
        case EM_AARCH64: {
            // mov w0 type
            // ret
            uint32_t* instptr = reinterpret_cast<uint32_t *>(getTypeCache.Real());
            /* 0xaa1f03e0 --> mov x0, xzr
             * 0x2a1f03e0 --> mov w0, wzr
             *
             * mov rd, #<imm>
             * |31|30 29|28 27 26 25 24 23|22 21| 20           5|4          0|
             * |sf| 0  0| 1  0  0  1  0  1|  hw | --- imm16 --- | --- Rd --- |
             *      opc
             *
             */
            int retry = 2;
            do {
                uint32_t inst = instptr[0];
                if (inst == 0xaa1f03e0 || inst == 0x2a1f03e0) {
                    type_cache = kSpaceTypeImageSpace;
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
                if (imm > kSpaceTypeRegionSpace) {
                    retry--;
                    instptr++;
                    continue;
                }

                type_cache = static_cast<SpaceType>(imm);
                break;
            } while (retry > 0);
        } break;
        case EM_ARM: {
            // thumb inst
            if (getTypeCache.Ptr() & 0x1) {
                // movs r0, type
                getTypeCache.MovePtr(-1);

                /*
                 * mov rd, #expr
                 * |15          11|10   8| 7            0|
                 * | 0| 0| 1| 0| 0|  rd  | -- immed_8 -- |
                 */

                uint16_t inst = *reinterpret_cast<uint16_t *>(getTypeCache.Real());
                uint32_t checkop = inst >> 11;
                uint32_t rd = (inst >> 8) & 0x7;
                uint32_t imm8 = inst & 0xFF;
                if (checkop != thumb::mov_expr || rd != 0x0
                        || imm8 > kSpaceTypeRegionSpace)
                    break;

                type_cache = static_cast<SpaceType>(imm8);
            }
        } break;
        case EM_RISCV:
            break;
        case EM_X86_64: {
            /*
             * mov $type,%rax  // xor %rax,%rax
             * retq
             */
            uint64_t inst = *reinterpret_cast<uint64_t *>(getTypeCache.Real());
            // xor
            if ((inst & 0xFFFF) == 0xc031) {
                type_cache = kSpaceTypeImageSpace;
            } else {
                inst = inst & 0xFFFFFFFFFFULL;
                uint8_t type = (inst >> 8) & 0xFF;
                if ((inst & 0xFF) != 0xb8 || type > kSpaceTypeRegionSpace)
                    break;

                type_cache = static_cast<SpaceType>(type);
            }
        } break;
        case EM_386: {
            uint64_t inst = *reinterpret_cast<uint64_t *>(getTypeCache.Real());
            if ((inst & 0xFFFF) == 0xc031) {
                type_cache = kSpaceTypeImageSpace;
            } else {
                inst = inst & 0xFFFFFFFFFFULL;
                if ((inst & 0xb8) == 0xb8) {
                    uint8_t type = (inst >> 8) & 0xFF;
                    if (!((inst & 0xFF) != 0xb8 || type > kSpaceTypeRegionSpace))
                        type_cache = static_cast<SpaceType>(type);
                }
            }

            if (type_cache != kSpaceTypeInvalidSpace)
                break;
            /*
             * <0>  push %ebp
             * <1>  mov %esp,%ebp
             * <3>  and $0xfffffffc,%esp
             * <6>  mov $type,%eax  // xor %eax,%eax
             * ...
             */
            inst = *reinterpret_cast<uint64_t *>(getTypeCache.Real() + 0x6);
            // xor
            if ((inst & 0xFFFF) == 0xc031) {
                type_cache = kSpaceTypeImageSpace;
            } else {
                inst = inst & 0xFFFFFFFFFFULL;
                if ((inst & 0xb8) == 0xb8) {
                    uint8_t type = (inst >> 8) & 0xFF;
                    if ((inst & 0xFF) != 0xb8 || type > kSpaceTypeRegionSpace)
                        break;
                    type_cache = static_cast<SpaceType>(type);
                }
            }
        } break;
    }

    if (type_cache != kSpaceTypeInvalidSpace)
        return type_cache;

    // second check space name
    const char* space_name = GetName();
    if (!space_name) return type_cache;

    if (!strcmp(space_name, REGION_SPACE)) {
        type_cache = kSpaceTypeRegionSpace;
    } else if (!strcmp(space_name, ZYGOTE_SPACE)) {
        type_cache = kSpaceTypeZygoteSpace;
    } else if (!strcmp(space_name, NON_MOVING_SPACE)
            || !strcmp(space_name, ROSALLOC_SPACE)) {
        type_cache = kSpaceTypeMallocSpace;
    } else if (!strcmp(space_name, FREELIST_SPACE)
            || !strcmp(space_name, MEMMAP_SPACE)) {
        type_cache = kSpaceTypeLargeObjectSpace;
    } else if (!strncmp(space_name, BUMP_POINTER_SPACE, strlen(BUMP_POINTER_SPACE))) {
        type_cache = kSpaceTypeBumpPointerSpace;
    } else {
        int len = strlen(space_name);
        if (len > 4 && !strncmp(space_name + len - 4, ".art", 4))
            type_cache = kSpaceTypeImageSpace;
    }
    return type_cache;
}

uint64_t ContinuousSpace::GetNextObject(mirror::Object& object) {
    const uint64_t position = object.Ptr() + object.SizeOf();
    return RoundUp(position, kObjectAlignment);
}

bool ContinuousSpace::IsVaildSpace() {
    return Begin() && CoreApi::IsVirtualValid(Begin());
}

} // namespace space
} // namespace gc
} // namespace art
