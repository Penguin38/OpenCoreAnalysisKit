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

#include "zip/zip_entry.h"
#include "logger/log.h"
#include <stdio.h>

status_t ZipEntry::initFromCDE(FILE* fp) {
    status_t result;
    off_t posn;
    result = mCDE.read(fp);
    if (result != 0) {
        LOGE("ERROR: read CDE failed\n");
        return result;
    }

    posn = ftell(fp);
    if (fseek(fp, mCDE.mLocalHeaderRelOffset, SEEK_SET) != 0) {
        LOGE("ERROR: localheader seek failed\n");
        return -1;
    }

    result = mLFH.read(fp);
    if (result != 0) {
        LOGE("ERROR: read LFH failed\n");
        return result;
    }

    if (fseek(fp, posn, SEEK_SET) != 0)
        return -1;
    return 0;
}

status_t ZipEntry::LocalFileHeader::read(FILE* fp) {
    status_t result = 0;
    uint8_t buf[kLFHLen];

    if (fread(buf, 1, kLFHLen, fp) != kLFHLen) {
        result = -1;
        goto bail;
    }

    if (ZipEntry::getLongLE(&buf[0x00]) != kSigNature) {
        LOGE("ERROR: didn't find the expected signature");
        result = -1;
        goto bail;
    }

    mVersion = ZipEntry::getShortLE(&buf[0x04]);
    mFlag = ZipEntry::getShortLE(&buf[0x06]);
    mCompressionMethod = ZipEntry::getShortLE(&buf[0x08]);
    mLastModTime = ZipEntry::getShortLE(&buf[0x0a]);
    mLastModDate = ZipEntry::getShortLE(&buf[0x0c]);
    mCRC = ZipEntry::getLongLE(&buf[0x0e]);
    mCompressedSize = ZipEntry::getLongLE(&buf[0x12]);
    mUncompressedSize = ZipEntry::getLongLE(&buf[0x16]);
    mFileNameLength = ZipEntry::getShortLE(&buf[0x1a]);
    mExtraLength = ZipEntry::getShortLE(&buf[0x1c]);

    if (mFileNameLength != 0) {
        mFileName = new uint8_t[mFileNameLength + 1];
        if (fread(mFileName, 1, mFileNameLength, fp) != mFileNameLength) {
            result = -1;
            goto bail;
        }
        mFileName[mFileNameLength] = '\0';
    }

    if (mExtraLength != 0) {
        mExtraField = new uint8_t[mExtraLength + 1];
        if (fread(mExtraField, 1, mExtraLength, fp) != mExtraLength) {
            result = -1;
            goto bail;
        }
        mExtraField[mExtraLength] = '\0';
    }

bail:
    return result;
}

status_t ZipEntry::CentralDirEntry::read(FILE* fp) {
    status_t result = 0;
    uint8_t buf[kCDELen];

    if (fread(buf, 1, kCDELen, fp) != kCDELen) {
        result = -1;
        goto bail;
    }

    if (ZipEntry::getLongLE(&buf[0x00]) != kSignature) {
        LOGE("ERROR: didn't find the expected signature");
        result = -1;
        goto bail;
    }

    mVersionMadeBy = ZipEntry::getShortLE(&buf[0x04]);
    mVersionToExtract = ZipEntry::getShortLE(&buf[0x06]);
    mGPBitFlag = ZipEntry::getShortLE(&buf[0x08]);
    mCompressionMethod = ZipEntry::getShortLE(&buf[0x0a]);
    mLastModFileTime = ZipEntry::getShortLE(&buf[0x0c]);
    mLastModFileDate = ZipEntry::getShortLE(&buf[0x0e]);
    mCRC32 = ZipEntry::getLongLE(&buf[0x10]);
    mCompressedSize = ZipEntry::getLongLE(&buf[0x14]);
    mUncompressedSize = ZipEntry::getLongLE(&buf[0x18]);
    mFileNameLength = ZipEntry::getShortLE(&buf[0x1c]);
    mExtraFieldLength = ZipEntry::getShortLE(&buf[0x1e]);
    mFileCommentLength = ZipEntry::getShortLE(&buf[0x20]);
    mDiskNumberStart = ZipEntry::getShortLE(&buf[0x22]);
    mInternalAttrs = ZipEntry::getShortLE(&buf[0x24]);
    mExternalAttrs = ZipEntry::getLongLE(&buf[0x26]);
    mLocalHeaderRelOffset = ZipEntry::getLongLE(&buf[0x2a]);

    if (mFileNameLength != 0) {
        mFileName = new uint8_t[mFileNameLength + 1];
        if (fread(mFileName, 1, mFileNameLength, fp) != mFileNameLength) {
            result = -1;
            goto bail;
        }
        mFileName[mFileNameLength] = '\0';
    }

    if (mExtraFieldLength != 0) {
        mExtraField = new uint8_t[mExtraFieldLength + 1];
        if (fread(mExtraField, 1, mExtraFieldLength, fp) != mExtraFieldLength) {
            result = -1;
            goto bail;
        }
        mFileName[mFileNameLength] = '\0';
    }

bail:
    return result;
}
