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

#ifndef UTILS_ZIP_ZIP_FILE_H_
#define UTILS_ZIP_ZIP_FILE_H_

#include "zip/zip_entry.h"
#include <vector>

class ZipFile {
public:
    ZipFile(void): mZipFp(NULL) {}
    ~ZipFile(void) {
        if (mZipFp != NULL)
            fclose(mZipFp);
        discardEntries();
    }
    status_t open(const char* zipFileName);
    status_t readCentralDir(void);
    void discardEntries(void);
    int getNumEntries(void) const { return mEntries.size(); }
    ZipEntry* getEntryByName(const char* fileName) const;

private:
    class EndOfCentralDir {
    public:
        EndOfCentralDir(void) :
            mDiskNumber(0),
            mDiskWithCentralDir(0),
            mNumEntries(0),
            mTotalNumEntries(0),
            mCentralDirSize(0),
            mCentralDirOffset(0),
            mCommentLen(0) {}
        virtual ~EndOfCentralDir(void) {}

        uint16_t mDiskNumber;
        uint16_t mDiskWithCentralDir;
        uint16_t mNumEntries;
        uint16_t mTotalNumEntries;
        uint32_t mCentralDirSize;
        uint32_t mCentralDirOffset;
        uint16_t mCommentLen;

        enum {
            kSignature = 0x06054b50,
            kEOCDLen = 22,
            kMaxCommentLen = 65535,
            kMaxEOCDSearch = kMaxCommentLen + EndOfCentralDir::kEOCDLen,
        };
        status_t readbuf(const uint8_t* buf, int len);
    };

    FILE* mZipFp;
    std::vector<ZipEntry*> mEntries;
    EndOfCentralDir mEOCD;
};

#endif  // UTILS_ZIP_ZIP_FILE_H_
