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

#ifndef ANDROID_ART_RUNTIME_OAT_OAT_FILE_H_
#define ANDROID_ART_RUNTIME_OAT_OAT_FILE_H_

#include "api/memory_ref.h"
#include "runtime/vdex_file.h"
#include "runtime/class_status.h"

struct OatFile_OffsetTable {
    uint32_t vdex_;
    uint32_t begin_;
    uint32_t is_executable_;
};
extern struct OatFile_OffsetTable __OatFile_offset__;

struct OatDexFile_OffsetTable {
    uint32_t oat_file_;
    uint32_t oat_class_offsets_pointer_;
};
extern struct OatDexFile_OffsetTable __OatDexFile_offset__;

namespace art {

class DexFile;

enum class OatClassType : uint8_t {
  kAllCompiled = 0,   // OatClass is followed by an OatMethodOffsets for each method.
  kSomeCompiled = 1,  // A bitmap of OatMethodOffsets that are present follows the OatClass.
  kNoneCompiled = 2,  // All methods are interpreted so no OatMethodOffsets are necessary.
  kLast = kNoneCompiled
};

class OatFile : public api::MemoryRef {
public:
    OatFile(uint64_t v) : api::MemoryRef(v) {}
    OatFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    OatFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    inline uint64_t vdex() { return VALUEOF(OatFile, vdex_); }
    inline uint64_t begin() { return VALUEOF(OatFile, begin_); }
    inline uint8_t is_executable() { return value8Of(OFFSET(OatFile, is_executable_)); }

    class OatMethod {
    public:
        uint32_t GetCodeOffset() const { return code_offset_; }
        OatMethod(const uint64_t base, const uint32_t code_offset)
            : begin_(base), code_offset_(code_offset) {}
        static const OatMethod Invalid() {
            return OatMethod(0, -1);
        }
        inline uint64_t GetQuickCode() {
            return code_offset_ ? begin_ + code_offset_ : 0x0;
        }
    private:
        uint64_t begin_;
        uint32_t code_offset_;
    };

    class OatClass {
    public:
        OatClass(uint64_t oat_file, ClassStatus status, OatClassType type,
                 uint32_t num_methods, uint64_t bitmap_pointer, uint64_t methods_pointer)
                : oat_file_(oat_file), status_(status), type_(type),
                  num_methods_(num_methods), bitmap_(bitmap_pointer), methods_pointer_(methods_pointer) {
        }
        static OatClass Invalid() {
            return OatClass(0, ClassStatus::kErrorUnresolved,
                            OatClassType::kNoneCompiled,
                            0, 0, 0);
        }

        OatMethod GetOatMethod(uint32_t method_index);
        api::MemoryRef GetOatMethodOffsets(uint32_t method_index);
    private:
        api::MemoryRef oat_file_;
        ClassStatus status_;
        OatClassType type_;
        uint32_t num_methods_;
        api::MemoryRef bitmap_;
        api::MemoryRef methods_pointer_;
    };

    VdexFile& GetVdexFile();
    uint64_t GetVdexBegin();
    static OatClass FindOatClass(DexFile& dex_file, uint16_t class_def_idx, bool* found);
private:
    // quick memoryref cache
    VdexFile vdex_cache = 0x0;
};

class OatDexFile : public api::MemoryRef {
public:
    OatDexFile(uint64_t v) : api::MemoryRef(v) {}
    OatDexFile(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    OatDexFile(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static void Init26();
    static void Init28();
    static void Init31();
    static void Init35();
    inline uint64_t oat_file() { return VALUEOF(OatDexFile, oat_file_); }
    inline uint64_t oat_class_offsets_pointer() { return VALUEOF(OatDexFile, oat_class_offsets_pointer_); }

    OatFile& GetOatFile();
    OatFile::OatClass GetOatClass(uint16_t class_def_index);
    inline bool IsBackedByVdexOnly() { return oat_class_offsets_pointer() == 0x0; }
private:
    // quick memoryref cache
    OatFile oat_file_cache = 0x0;
};

} // namespace art

#endif // ANDROID_ART_RUNTIME_OAT_OAT_FILE_H_
