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
#include "api/core.h"
#include "base/utils.h"
#include "common/native_frame.h"
#include "common/file.h"
#include "common/elf.h"

NativeFrame::NativeFrame(uint64_t fp, uint64_t sp, uint64_t pc)
        : frame_fp(fp), frame_sp(sp), map(nullptr), thumb(false) {
    SetFramePc(pc);
}

void NativeFrame::Decode() {
    LoadBlock* block = CoreApi::FindLoadBlock(frame_pc, false);

    auto callback = [&](LinkMap* link) -> bool {
        // FOR TEST
        uint64_t va_pc = frame_pc & CoreApi::GetVabitsMask();
        LoadBlock* ld_block = CoreApi::FindLoadBlock(link->l_ld(), false);
        if (block && block->filename().length()
                && block->filename() == link->name()) {
            map = link;
            return true;
        } else if (link->begin() && va_pc >= link->begin()
                && (va_pc < link->l_ld() ||
                    (ld_block ? (va_pc <= ld_block->vaddr() + ld_block->size()) : false))) {
            map = link;
            return true;

        }
        return false;
    };

    if (block && block->handle()) {
        map = block->handle();
    } else {
        CoreApi::ForeachLinkMap(callback);
    }
    if (map) map->NiceMethod(frame_pc, frame_symbol);
}

std::string NativeFrame::GetLibrary() {
    if (!map) {
        return "";
    }

    std::string name = map->name();
    LoadBlock* block = map->block();
    if (block && block->isMmapBlock()) {
        name = block->name();
        return name;
    }

    File* file = CoreApi::FindFile(frame_pc);
    name = file? file->name() : "";
    return name;
}

uint64_t NativeFrame::GetMethodOffset() {
    return frame_symbol.GetOffset();
}

uint64_t NativeFrame::GetMethodSize() {
    return frame_symbol.GetSize();
}

void NativeFrame::SetFramePc(uint64_t pc) {
    if (CoreApi::GetMachine() == EM_ARM) {
        frame_pc = pc & (CoreApi::GetPointMask() - 1);
    } else {
        frame_pc = pc;
    }
}

bool NativeFrame::IsThumbMode() {
    return thumb;
}
