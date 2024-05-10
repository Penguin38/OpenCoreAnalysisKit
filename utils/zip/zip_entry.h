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

#ifndef UTILS_ZIP_ZIP_ENTRY_H_
#define UTILS_ZIP_ZIP_ENTRY_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

typedef int32_t status_t;

class ZipEntry {
public:

    /* defined for Zip archives */
    enum {
        kCompressStored     = 0,        // no compression
        // shrunk           = 1,
        // reduced 1        = 2,
        // reduced 2        = 3,
        // reduced 3        = 4,
        // reduced 4        = 5,
        // imploded         = 6,
        // tokenized        = 7,
        kCompressDeflated   = 8,        // standard deflate
        // Deflate64        = 9,
        // lib imploded     = 10,
        // reserved         = 11,
        // bzip2            = 12,
    };

    friend class ZipFile;

    ZipEntry(void) {}
    ~ZipEntry(void) {}

    static inline uint16_t getShortLE(const uint8_t* buf) {
        return buf[0] | (buf[1] << 8);
    }

    static inline uint32_t getLongLE(const uint8_t* buf) {
        return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    }

    const char* getFileName(void) const { return (const char*) mCDE.mFileName; }

    off_t getFileOffset(void) const {
        return mCDE.mLocalHeaderRelOffset +
            LocalFileHeader::kLFHLen +
            mLFH.mFileNameLength +
            mLFH.mExtraLength;
    }

    off_t getLHFOffset(void) const { return mCDE.mLocalHeaderRelOffset; }

    off_t getEntryTotalMemsz() const {
        return LocalFileHeader::kLFHLen +
            mLFH.mFileNameLength +
            mLFH.mExtraLength +
            mLFH.mCompressedSize;
    }

    bool IsUncompressed(void) const {
        return mCDE.mCompressionMethod == kCompressStored;
    }

protected:
    status_t initFromCDE(FILE* fp);

private:
    class LocalFileHeader {
    public:
        LocalFileHeader(void) :
            mVersion(0),
            mFlag(0),
            mCompressionMethod(0),
            mLastModTime(0),
            mLastModDate(0),
            mCRC(0),
            mCompressedSize(0),
            mUncompressedSize(0),
            mFileNameLength(0),
            mExtraLength(0),
            mFileName(NULL),
            mExtraField(NULL) {}

        virtual ~LocalFileHeader(void) {
            delete[] mFileName;
            delete[] mExtraField;
        }

        status_t read(FILE* fp);

        //uint32_t mSignature;
        uint16_t mVersion;
        uint16_t mFlag;
        uint16_t mCompressionMethod;
        uint16_t mLastModTime;
        uint16_t mLastModDate;
        uint32_t mCRC;
        uint32_t mCompressedSize;
        uint32_t mUncompressedSize;
        uint16_t mFileNameLength;
        uint16_t mExtraLength;

        uint8_t* mFileName;
        uint8_t* mExtraField;

        enum {
            kSigNature = 0x04034b50,
            kLFHLen      = 30,
        };
    };

    class CentralDirEntry{
    public:
        CentralDirEntry(void):
            mVersionMadeBy(0),
            mVersionToExtract(0),
            mGPBitFlag(0),
            mCompressionMethod(0),
            mLastModFileTime(0),
            mLastModFileDate(0),
            mCRC32(0),
            mCompressedSize(0),
            mUncompressedSize(0),
            mFileNameLength(0),
            mExtraFieldLength(0),
            mFileCommentLength(0),
            mDiskNumberStart(0),
            mInternalAttrs(0),
            mExternalAttrs(0),
            mLocalHeaderRelOffset(0),
            mFileName(NULL),
            mExtraField(NULL),
            mFileComment(NULL) {}

        ~CentralDirEntry(void){
            delete[] mFileName;
            delete[] mExtraField;
            delete[] mFileComment;
        }

        status_t read(FILE* fp);

        // uint32_t mSigNature;
        uint16_t mVersionMadeBy;
        uint16_t mVersionToExtract;
        uint16_t mGPBitFlag;
        uint16_t mCompressionMethod;
        uint16_t mLastModFileTime;
        uint16_t mLastModFileDate;
        uint32_t mCRC32;
        uint32_t mCompressedSize;
        uint32_t mUncompressedSize;
        uint16_t mFileNameLength;
        uint16_t mExtraFieldLength;
        uint16_t mFileCommentLength;
        uint16_t mDiskNumberStart;
        uint16_t mInternalAttrs;
        uint32_t mExternalAttrs;
        uint32_t mLocalHeaderRelOffset;
        uint8_t* mFileName;
        uint8_t* mExtraField;
        uint8_t* mFileComment;

        enum {
            kSignature = 0x02014b50,
            kCDELen     = 46,
        };
    };

    CentralDirEntry mCDE;
    LocalFileHeader mLFH;
};

#endif  // UTILS_ZIP_ZIP_ENTRY_H_
