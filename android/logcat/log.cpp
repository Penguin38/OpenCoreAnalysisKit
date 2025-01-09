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

#include "logcat/log.h"
#include "api/core.h"
#include "common/auxv.h"
#include "android.h"
#include "logcat/LogBuffer.h"
#include "logcat/LogStatistics.h"
#include "logcat/SerializedData.h"
#include "logcat/SerializedLogBuffer.h"

namespace android {

void Logcat::Init() {
    android::LogBuffer::Init();
    android::LogStatistics::Init();
    android::SerializedData::Init();

    Android::RegisterSdkListener(Android::S, android::SerializedLogBuffer::Init31);
}

/*
 *      Scan VMA         SerializedLogBuffer         Core
 *   --------------    -->  -------------        --------------
 *   |            |    |    |   vtbl    |----    |            |
 *   |------------|test|    |-----------|   |in  |------------|
 *   |  VMA (RW)  |-----    |-----------|   ---> | logd .text |
 *   |------------|  |    --|reader_list|        |------------|
 *   |  VMA (RW)  |---  --| |   tags    |        |            |
 *   |------------|     | --|   stats   |        |------------|
 *   |            |     |   |-----------|   ---> |    stack   |
 *   --------------     |   -------------   |in  --------------
 *                      |--------------------
 */
SerializedLogBuffer Logcat::AnalysisSerializedLogBuffer() {
    SerializedLogBuffer serial = 0x0;
    api::MemoryRef exec_text = CoreApi::FindAuxv(AT_ENTRY);
    api::MemoryRef exec_fn = CoreApi::FindAuxv(AT_EXECFN);
    api::MemoryRef platform = CoreApi::FindAuxv(AT_PLATFORM);
    exec_text.Prepare(false);
    exec_fn.Prepare(false);
    platform.Prepare(false);

    if (exec_fn.IsValid()) {
        std::string name = reinterpret_cast<const char*>(exec_fn.Real());
        if (name.length() > 0 && name != "/system/bin/logd") {
            LOGE("Exec filename \"%s\" not that \"/system/bin/logd\".\n", name.c_str());
            return false;
        }
    }
    uint32_t point_size = CoreApi::GetPointSize();
    auto callback = [&](LoadBlock *block) -> bool {
        if (!(block->flags() & Block::FLAG_W))
            return false;

        SerializedLogBuffer buffer(block->vaddr(), block);
        do {
            api::MemoryRef vtbl = buffer.valueOf();
            if (vtbl.IsValid()) {
                bool match = true;
                // virtual method
                for (int k = 0; k < MEMBER_SIZE(SerializedLogBuffer, vtbl); ++k) {
                    if (!exec_text.Block()->virtualContains(vtbl.valueOf(k * point_size))) {
                        match = false;
                        break;
                    }
                }

                if (match) {
                    match &= platform.Block()->virtualContains(buffer.reader_list());
                    match &= platform.Block()->virtualContains(buffer.tags());
                    match &= platform.Block()->virtualContains(buffer.stats());
                }

                if (match) {
                    serial = buffer;
                    serial.copyRef(buffer);
                    return true;
                }
            }

            buffer.MovePtr(point_size);
        } while (buffer.Ptr() + SIZEOF(SerializedLogBuffer) < block->vaddr() + block->size());
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, true, true);
    return serial;
}

} // namespace android
