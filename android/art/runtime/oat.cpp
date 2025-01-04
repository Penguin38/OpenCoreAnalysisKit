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
#include "api/memory_ref.h"
#include "common/elf.h"
#include "android.h"
#include <linux/elf.h>
#include <string>

namespace art {

int OatHeader::OatVersion() {
    OatHeader& header = Android::GetOatHeader();
    if (!header.kOatVersion) {
        api::MemoryRef version = Android::DlSym(Android::ART_OAT_HEADER_VERSION);
        if (version.Ptr()) {
            char kOatVersion[4];
            kOatVersion[0] = version.value8Of(0);
            kOatVersion[1] = version.value8Of(1);
            kOatVersion[2] = version.value8Of(2);
            kOatVersion[3] = version.value8Of(3);
            header.kOatVersion = std::stoi(kOatVersion);
        } else {
            header.kOatVersion = AnalysiOatVersion();
        }
    }
    return header.kOatVersion;
}

int OatHeader::AnalysiOatVersion() {
    uint8_t oat_magic[4] = {0x6F, 0x61, 0x74, 0x0A}; // oat\n
    char kOatVersion[4];
    int found_version = 0;

    uint64_t point_size = CoreApi::GetPointSize();
    auto callback = [&](LoadBlock *block) -> bool {
        ElfHeader* header = reinterpret_cast<ElfHeader*>(block->begin());
        if (memcmp(header->ident, ELFMAG, 4)) {
            return false;
        }

        api::MemoryRef current(block->vaddr(), block);
        uint64_t outsize = block->vaddr() + block->size();
        uint64_t padding_offset = sizeof(kOatVersion) + sizeof(oat_magic);
        while (current.Ptr() + padding_offset <= outsize) {
            if (!memcmp(reinterpret_cast<void *>(current.Real()),
                        reinterpret_cast<void *>(oat_magic),
                        sizeof(oat_magic))) {

                api::MemoryRef version(current.Ptr() + sizeof(oat_magic), block);
                kOatVersion[0] = version.value8Of(0);
                kOatVersion[1] = version.value8Of(1);
                kOatVersion[2] = version.value8Of(2);
                kOatVersion[3] = version.value8Of(3);

                if (kOatVersion[0] >= 0x30 && kOatVersion[0] <= 0x39
                        && kOatVersion[1] >= 0x30 && kOatVersion[1] <= 0x39
                        && kOatVersion[2] >= 0x30 && kOatVersion[2] <= 0x39
                        && kOatVersion[3] == 0x0) {
                    found_version = std::stoi(kOatVersion);
                    LOGD(">>> \'%s\' = %d\n", Android::ART_OAT_HEADER_VERSION, found_version);
                    return true;
                }
            }
            current.MovePtr(point_size);
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, true, true);
    return found_version;
}

} // namespace art
