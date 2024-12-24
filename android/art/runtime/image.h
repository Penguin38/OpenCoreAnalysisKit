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

#ifndef ANDROID_ART_RUNTIME_IMAGE_H_
#define ANDROID_ART_RUNTIME_IMAGE_H_

#include "logger/log.h"
#include "api/memory_ref.h"

struct ImageHeader_OffsetTable {
    uint32_t image_methods_;
};

struct ImageHeader_SizeTable {
    uint32_t THIS;
};

extern struct ImageHeader_OffsetTable __ImageHeader_offset__;
extern struct ImageHeader_SizeTable __ImageHeader_size__;

namespace art {

class ImageHeader : public api::MemoryRef {
public:
    enum ImageMethod {
        kResolutionMethod,
        kImtConflictMethod,
        kImtUnimplementedMethod,
        kSaveAllCalleeSavesMethod,
        kSaveRefsOnlyMethod,
        kSaveRefsAndArgsMethod, // 24
        kSaveEverythingMethod,  // 26 ~ 27
        kSaveEverythingMethodForClinit,
        kSaveEverythingMethodForSuspendCheck,
        // kImageMethodsCount,  // Number of elements in enum.
    };

    ImageHeader(uint64_t v) : api::MemoryRef(v) {}
    ImageHeader(uint64_t v, LoadBlock* b) : api::MemoryRef(v, b) {}
    ImageHeader(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    ImageHeader(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    ImageHeader(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    inline bool operator==(ImageHeader& ref) { return Ptr() == ref.Ptr(); }
    inline bool operator!=(ImageHeader& ref) { return Ptr() != ref.Ptr(); }

    static inline uint8_t kMagic[4] = {0x61, 0x72, 0x74, 0x0A}; // art\n
    static void Init();
    static void Init24();
    static void Init26();
    static void Init28();
    static void Init29();
    static void Init30();
    static void Init31();
    static void Init34();
    static void Init35();
    inline uint64_t image_methods() { return Real() + OFFSET(ImageHeader, image_methods_); }

    inline uint32_t GetImageMethodsCount() { return kNumImageMethodsCount; }
private:
    static uint32_t kNumImageMethodsCount;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_IMAGE_H_
