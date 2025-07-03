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

#include "arm64/core.h"
#include "arm64/thread_info.h"
#include "common/prstatus.h"
#include "common/fpregset.h"
#include "common/tls.h"
#include "common/bit.h"
#include "common/elf.h"
#include <string.h>
#include <linux/elf.h>

namespace arm64 {

struct user_pac_mask {
    uint64_t data_mask;
    uint64_t insn_mask;
};

bool Core::load() {
    pointer_mask = GENMASK_UL(bits() - 1, 0);
    vabits_mask = GENMASK_UL((VA_BITS ? VA_BITS : DEF_VA_BITS) - 1, 0);

    auto callback = [this](OptionArgs& args) -> void * {
        switch(args.type) {
            case NT_PRSTATUS: {
                Elf64_prstatus* prs = reinterpret_cast<Elf64_prstatus *>(args.pos);
                ThreadInfo* thread = new ThreadInfo(prs->pr_pid, args.pos);
                memcpy(&thread->reg, &prs->pr_reg, sizeof(Register));
                return thread;
            } break;
            case NT_FPREGSET: {
                if (args.context) {
                    Elf64_fpregset* fpr = reinterpret_cast<Elf64_fpregset *>(args.pos);
                    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(args.context);
                    memcpy(&thread->fpr, fpr, sizeof(FpRegister));
                }
            } break;
            case NT_ARM_TLS: {
                if (args.context) {
                    Elf64_tls* tls = reinterpret_cast<Elf64_tls *>(args.pos);
                    ThreadInfo* thread = reinterpret_cast<ThreadInfo*>(args.context);
                    memcpy(&thread->tls, tls, sizeof(Tls));
                }
            } break;
            case NT_ARM_PAC_MASK: {
                user_pac_mask* uregs = reinterpret_cast<user_pac_mask *>(args.pos);
                data_mask = uregs->data_mask;
                insn_mask = uregs->insn_mask;
                // pointer_mask = (1ULL << (63 - __builtin_clzll(data_mask))) - 1;
                vabits_mask = GENMASK_UL(__builtin_ctzll(data_mask) - 1, 0);
            } break;
            case NT_ARM_TAGGED_ADDR_CTRL: {
                tagged_addr_ctrl = *reinterpret_cast<uint64_t *>(args.pos);
            } break;
            case NT_ARM_PAC_ENABLED_KEYS: {
                pac_enabled_keys = *reinterpret_cast<uint64_t *>(args.pos);
            } break;
        }
        return nullptr;
    };
    return load64(this, callback);
}

void Core::unload() {
}

Core::~Core() {
    unload();
}

} // namespace arm64
