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

#include "zip/zip_file.h"
#include "logger/log.h"
#include <string.h>

status_t ZipFile::open(const char* zipFileName) {
    mZipFp = fopen(zipFileName, "rb");
    if (mZipFp == NULL) {
        LOGE("fopen failed %s\n", zipFileName);
        return -1;
    }
    status_t result;
    result = readCentralDir();
    return result;
}

status_t ZipFile::readCentralDir(void) {
    status_t result = 0;
    uint8_t* buf = NULL;
    long fileLength, seekStart, readAmount;

    fseek(mZipFp, 0, SEEK_END);
    fileLength = ftell(mZipFp);
    rewind(mZipFp);

    if (fileLength < EndOfCentralDir::kEOCDLen) {
        LOGE("Length is %ld -- too small\n", (long)fileLength);
        return -1;
    }

    buf = new uint8_t[EndOfCentralDir::kMaxEOCDSearch];
    if (buf == NULL) {
        LOGE("Failure allocating %d bytes for EOCD search",
                EndOfCentralDir::kMaxEOCDSearch);
        result = -1;
        goto bail;
    }

    if (fileLength > EndOfCentralDir::kMaxEOCDSearch) {
        seekStart = fileLength - EndOfCentralDir::kMaxEOCDSearch;
        readAmount = EndOfCentralDir::kMaxEOCDSearch;
    } else {
        seekStart = 0;
        readAmount = fileLength;
    }

    if (fseek(mZipFp, seekStart, SEEK_SET) != 0) {
        LOGE("Failure seeking to end of zip at %ld", (long) seekStart);
        result = -1;
        goto bail;
    }

    if (fread(buf, 1, readAmount, mZipFp) != readAmount) {
        result = -1;
        goto bail;
    }

    int i;
    for (i = readAmount - 4; i >=0; i--) {
        if (buf[i] == 0x50 && ZipEntry::getLongLE(&buf[i]) == EndOfCentralDir::kSignature) {
            break;
        }
    }

    if (i < 0) {
        LOGE("EOCD not found, not Zip\n");
        result = -1;
        goto bail;
    }

    result = mEOCD.readbuf(buf + i, readAmount - i);
    if (result != 0) {
        LOGE("Failure reading %ld bytes of EOCD values", readAmount - i);
        goto bail;
    }

    if (mEOCD.mDiskNumber != 0 || mEOCD.mDiskWithCentralDir != 0 ||
            mEOCD.mNumEntries != mEOCD.mTotalNumEntries) {
        LOGE("Archive spanning not supported\n");
        result = -1;
        goto bail;
    }

    if (fseek(mZipFp, mEOCD.mCentralDirOffset, SEEK_SET) != 0) {
        LOGE("failed to seek mCentralDirOffset\n");
        result = -1;
        goto bail;
    }

    int entry;
    for (entry = 0; entry < mEOCD.mTotalNumEntries; entry++) {
        ZipEntry* pEntry = new ZipEntry;
        result = pEntry->initFromCDE(mZipFp);
        if (result != 0) {
            LOGE("initFromCDE failed\n");
            delete pEntry;
            goto bail;
        }
        mEntries.push_back(pEntry);
    }

    {
        uint8_t checkBuf[4];
        if (fread(checkBuf, 1, 4, mZipFp) != 4) {
            result = -1;
            goto bail;
        }

        if (ZipEntry::getLongLE(checkBuf) != EndOfCentralDir::kSignature) {
            LOGE("EOCD read check failed\n");
            result = -1;
            goto bail;
        }
    }

bail:
    delete[] buf;
    return result;
}

void ZipFile::discardEntries(void) {
    int count = mEntries.size();
    while (--count >= 0)
        delete mEntries[count];

    mEntries.clear();
}

status_t ZipFile::EndOfCentralDir::readbuf(const uint8_t* buf, int len) {
    if (len < kEOCDLen) {
        LOGE("zip eocd truncated\n");
    }
    if (ZipEntry::getLongLE(&buf[0x00]) != kSignature) {
        return -1;
    }
    mDiskNumber = ZipEntry::getShortLE(&buf[0x04]);
    mDiskWithCentralDir = ZipEntry::getShortLE(&buf[0x06]);
    mNumEntries = ZipEntry::getShortLE(&buf[0x08]);
    mTotalNumEntries = ZipEntry::getShortLE(&buf[0x0a]);
    mCentralDirSize = ZipEntry::getLongLE(&buf[0x0c]);
    mCentralDirOffset = ZipEntry::getLongLE(&buf[0x10]);
    mCommentLen = ZipEntry::getShortLE(&buf[0x14]);
    return 0;
}

ZipEntry* ZipFile::getEntryByName(const char* fileName) const {
    for (int i = mEntries.size()-1; i >= 0; i--) {
        ZipEntry* pEntry = mEntries[i];
        if (strcmp(fileName, pEntry->getFileName()) == 0) {
            return pEntry;
        }
    }
    return NULL;
}
