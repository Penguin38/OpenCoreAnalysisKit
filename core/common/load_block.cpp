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

#include "common/load_block.h"

uint64_t LoadBlock::begin() {
    if (mOverlayAddr)
        return mOverlayAddr;
    if (mMmapAddr)
        return mMmapAddr;
    if (mOriAddr)
        return mOriAddr;
    return 0x0;
}

bool LoadBlock::isValid() {
    if ((mFlags & FLAG_R) && (mFileSize || mMmapAddr || mOverlayAddr))
        return true;
    return false;
}

bool LoadBlock::virtualContains(uint64_t vaddr) {
    if (vaddr >= mVaddr && vaddr < (mVaddr + mMemSize))
        return true;
    return false;
}

bool LoadBlock::realContains(uint64_t raddr) {
    if (mOverlayAddr) {
        return (raddr >= mOverlayAddr && raddr < (mOverlayAddr + mMemSize));
    } else if (mMmapAddr) {
        return (raddr >= mMmapAddr && raddr < (mMmapAddr + mMemSize));
    } else if (mOriAddr) {
        return (raddr >= mOriAddr && raddr < (mOriAddr + mMemSize));
    }
    return false;
}
