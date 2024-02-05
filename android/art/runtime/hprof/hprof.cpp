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

#include "runtime/hprof/hprof.h"
#include "runtime/runtime.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/space.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/array.h"
#include "runtime/runtime_globals.h"
#include "android.h"
#include "cxx/vector.h"
#include <stdio.h>

namespace art {
namespace hprof {

#define HLOGV(...) \
do { \
    if (first_ && visible_) LOGI(__VA_ARGS__); \
} while(0)

static constexpr uint32_t kHprofTime = 0;
static constexpr uint32_t kHprofNullThread = 0;

static constexpr size_t kMaxObjectsPerSegment = 128;
static constexpr size_t kMaxBytesPerSegment = 4096;

// The static field-name for the synthetic object generated to account for class static overhead.
static constexpr const char* kClassOverheadName = "$classOverhead";

enum HprofTag {
    HPROF_TAG_STRING = 0x01,
    HPROF_TAG_LOAD_CLASS = 0x02,
    HPROF_TAG_UNLOAD_CLASS = 0x03,
    HPROF_TAG_STACK_FRAME = 0x04,
    HPROF_TAG_STACK_TRACE = 0x05,
    HPROF_TAG_ALLOC_SITES = 0x06,
    HPROF_TAG_HEAP_SUMMARY = 0x07,
    HPROF_TAG_START_THREAD = 0x0A,
    HPROF_TAG_END_THREAD = 0x0B,
    HPROF_TAG_HEAP_DUMP = 0x0C,
    HPROF_TAG_HEAP_DUMP_SEGMENT = 0x1C,
    HPROF_TAG_HEAP_DUMP_END = 0x2C,
    HPROF_TAG_CPU_SAMPLES = 0x0D,
    HPROF_TAG_CONTROL_SETTINGS = 0x0E,
};

// Values for the first byte of HEAP_DUMP and HEAP_DUMP_SEGMENT records:
enum HprofHeapTag {
    // Traditional.
    HPROF_ROOT_UNKNOWN = 0xFF,
    HPROF_ROOT_JNI_GLOBAL = 0x01,
    HPROF_ROOT_JNI_LOCAL = 0x02,
    HPROF_ROOT_JAVA_FRAME = 0x03,
    HPROF_ROOT_NATIVE_STACK = 0x04,
    HPROF_ROOT_STICKY_CLASS = 0x05,
    HPROF_ROOT_THREAD_BLOCK = 0x06,
    HPROF_ROOT_MONITOR_USED = 0x07,
    HPROF_ROOT_THREAD_OBJECT = 0x08,
    HPROF_CLASS_DUMP = 0x20,
    HPROF_INSTANCE_DUMP = 0x21,
    HPROF_OBJECT_ARRAY_DUMP = 0x22,
    HPROF_PRIMITIVE_ARRAY_DUMP = 0x23,

    // Android.
    HPROF_HEAP_DUMP_INFO = 0xfe,
    HPROF_ROOT_INTERNED_STRING = 0x89,
    HPROF_ROOT_FINALIZING = 0x8a,  // Obsolete.
    HPROF_ROOT_DEBUGGER = 0x8b,
    HPROF_ROOT_REFERENCE_CLEANUP = 0x8c,  // Obsolete.
    HPROF_ROOT_VM_INTERNAL = 0x8d,
    HPROF_ROOT_JNI_MONITOR = 0x8e,
    HPROF_UNREACHABLE = 0x90,  // Obsolete.
    HPROF_PRIMITIVE_ARRAY_NODATA_DUMP = 0xc3,  // Obsolete.
};

enum HprofHeapId {
    HPROF_HEAP_DEFAULT = 0,
    HPROF_HEAP_ZYGOTE = 'Z',
    HPROF_HEAP_APP = 'A',
    HPROF_HEAP_IMAGE = 'I',
};

using HprofStringId = uint32_t;
using HprofClassObjectId = uint32_t;
using HprofClassSerialNumber = uint32_t;
using HprofStackTraceSerialNumber = uint32_t;
using HprofStackFrameId = uint32_t;
static constexpr HprofStackTraceSerialNumber kHprofNullStackTrace = 0;

mirror::Object HprofInvalidObject = 0x0;

class EndianOutput {
public:
    EndianOutput() : length_(0), sum_length_(0), max_length_(0), started_(false) {}
    virtual ~EndianOutput() {}

    void StartNewRecord(uint8_t tag, uint32_t time) {
        if (length_ > 0) {
            EndRecord();
        }

        AddU1(tag);
        AddU4(time);
        AddU4(0xdeaddead);  // Length, replaced on flush.
        started_ = true;
    }

    void EndRecord() {
        // Replace length in header.
        if (started_) {
            UpdateU4(sizeof(uint8_t) + sizeof(uint32_t),
                    length_ - sizeof(uint8_t) - 2 * sizeof(uint32_t));
        }

        HandleEndRecord();

        sum_length_ += length_;
        max_length_ = std::max(max_length_, length_);
        length_ = 0;
        started_ = false;
    }

    void AddU1(uint8_t value) {
        AddU1List(&value, 1);
    }
    void AddU2(uint16_t value) {
        AddU2List(&value, 1);
    }
    void AddU4(uint32_t value) {
        AddU4List(&value, 1);
    }

    void AddU8(uint64_t value) {
        AddU8List(&value, 1);
    }

    void AddObjectId(mirror::Object& value) {
        AddU4(value.Ptr());
    }

    void AddStackTraceSerialNumber(HprofStackTraceSerialNumber value) {
        AddU4(value);
    }

    // The ID for the synthetic object generated to account for class static overhead.
    void AddClassStaticsId(mirror::Class& value) {
        AddU4(1 | value.Ptr());
    }

    void AddClassId(HprofClassObjectId value) {
        AddU4(value);
    }

    void AddStringId(HprofStringId value) {
        AddU4(value);
    }

    void AddU1List(const uint8_t* values, size_t count) {
        HandleU1List(values, count);
        length_ += count;
    }

    void AddU2List(const uint16_t* values, size_t count) {
        HandleU2List(values, count);
        length_ += count * sizeof(uint16_t);
    }

    void AddU4List(const uint32_t* values, size_t count) {
        HandleU4List(values, count);
        length_ += count * sizeof(uint32_t);
    }

    virtual void UpdateU4(size_t offset, uint32_t new_value) {}

    void AddU8List(const uint64_t* values, size_t count) {
        HandleU8List(values, count);
        length_ += count * sizeof(uint64_t);
    }

    void AddIdList(mirror::Array& values) {
        const int32_t length = values.GetLength();
        for (int32_t i = 0; i < length; ++i) {
            api::MemoryRef ref(values.GetRawData(sizeof(uint32_t), i), values);
            art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), values);
            AddObjectId(object);
        }
    }

    void AddUtf8String(const char* str) {
        // The terminating NUL character is NOT written.
        AddU1List((const uint8_t*)str, strlen(str));
    }

    size_t Length() const {
        return length_;
    }

    size_t SumLength() const {
        return sum_length_;
    }

    size_t MaxLength() const {
        return max_length_;
    }

protected:
    virtual void HandleU1List(const uint8_t* values, size_t count) {
    }
    virtual void HandleU1AsU2List(const uint8_t* values,
            size_t count) {}
    virtual void HandleU2List(const uint16_t* values,
            size_t count) {}
    virtual void HandleU4List(const uint32_t* values,
            size_t count) {}
    virtual void HandleU8List(const uint64_t* values,
            size_t count) {}
    virtual void HandleEndRecord() {
    }

    size_t length_;      // Current record size.
    size_t sum_length_;  // Size of all data.
    size_t max_length_;  // Maximum seen length.
    bool started_;       // Was StartRecord called?
};

// This keeps things buffered until flushed.
class EndianOutputBuffered : public EndianOutput {
public:
    explicit EndianOutputBuffered(size_t reserve_size) {
        buffer_.reserve(reserve_size);
    }
    virtual ~EndianOutputBuffered() {}

    void UpdateU4(size_t offset, uint32_t new_value) override {
        buffer_[offset + 0] = static_cast<uint8_t>((new_value >> 24) & 0xFF);
        buffer_[offset + 1] = static_cast<uint8_t>((new_value >> 16) & 0xFF);
        buffer_[offset + 2] = static_cast<uint8_t>((new_value >> 8)  & 0xFF);
        buffer_[offset + 3] = static_cast<uint8_t>((new_value >> 0)  & 0xFF);
    }

protected:
    void HandleU1List(const uint8_t* values, size_t count) override {
        buffer_.insert(buffer_.end(), values, values + count);
    }

    void HandleU1AsU2List(const uint8_t* values, size_t count) override {
        // All 8-bits are grouped in 2 to make 16-bit block like Java Char
        if (count & 1) {
            buffer_.push_back(0);
        }
        for (size_t i = 0; i < count; ++i) {
            uint8_t value = *values;
            buffer_.push_back(value);
            values++;
        }
    }

    void HandleU2List(const uint16_t* values, size_t count) override {
        for (size_t i = 0; i < count; ++i) {
            uint16_t value = *values;
            buffer_.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 0) & 0xFF));
            values++;
        }
    }

    void HandleU4List(const uint32_t* values, size_t count) override {
        for (size_t i = 0; i < count; ++i) {
            uint32_t value = *values;
            buffer_.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 8)  & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 0)  & 0xFF));
            values++;
        }
    }

    void HandleU8List(const uint64_t* values, size_t count) override {
        for (size_t i = 0; i < count; ++i) {
            uint64_t value = *values;
            buffer_.push_back(static_cast<uint8_t>((value >> 56) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 48) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 40) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 32) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 8)  & 0xFF));
            buffer_.push_back(static_cast<uint8_t>((value >> 0)  & 0xFF));
            values++;
        }
    }

    void HandleEndRecord() override {
        HandleFlush(buffer_.data(), length_);
        buffer_.clear();
    }

    virtual void HandleFlush(const uint8_t* buffer, size_t length) {
    }

    std::vector<uint8_t> buffer_;
};

class FileEndianOutput final : public EndianOutputBuffered {
public:
    FileEndianOutput(FILE* fp, size_t reserved_size)
        : EndianOutputBuffered(reserved_size), fp_(fp), errors_(false) {
        }

    ~FileEndianOutput() {
    }

    bool Errors() {
        return errors_;
    }

protected:
  void HandleFlush(const uint8_t* buffer, size_t length) override {
      if (!errors_) {
          errors_ = !fwrite(buffer, length, 1, fp_);
      }
  }

private:
  FILE* fp_;
  bool errors_;
};

#define __ output_->

class Hprof {
public:
    Hprof(const char* output, bool visible) : filename_(output)
            ,visible_(visible), first_(true) {}

    void Dump() {
        LOGI("hprof: heap dump %s starting...\n", filename_);

        // First pass to measure the size of the dump.
        size_t max_length;
        {
            EndianOutput count_output;
            output_ = &count_output;
            ProcessHeap(false);
            max_length = count_output.MaxLength();
            output_ = nullptr;
        }

        first_ = false;
        bool okay = DumpToFile(max_length);
        if (okay)
            LOGI("hprof: heap dump completed. objects (%lu)\n", total_objects_);
    }

private:
    bool DumpHeapObject(mirror::Object& object);
    void DumpHeapClass(mirror::Class& klass);
    void DumpHeapArray(mirror::Array& array, mirror::Class& klass);
    void DumpFakeObjectArray(mirror::Object& object);
    void DumpHeapInstanceObject(mirror::Object& object, mirror::Class& klass);
    bool AddRuntimeInternalObjectsField(mirror::Class& klass);

    bool DumpToFile(size_t max_length) {
        FILE *fp = fopen(filename_, "wb");
        if (!fp)
            return false;

        FileEndianOutput file_output(fp, max_length);
        output_ = &file_output;
        ProcessHeap(true);
        fclose(fp);
        return true;
    }

    void ProcessHeap(bool header_first) {
        current_heap_ = HPROF_HEAP_DEFAULT;
        objects_in_segment_ = 0;
        if (header_first) {
            ProcessHeader(true);
            ProcessBody();
        } else {
            ProcessBody();
            ProcessHeader(false);
        }
    }

    void ProcessBody() {
        // Walk the roots and the heap.
        output_->StartNewRecord(HPROF_TAG_HEAP_DUMP_SEGMENT, 0x0);
        auto callback = [&](art::mirror::Object& object) -> bool {
            return DumpHeapObject(object);
        };
        Android::ForeachObjects(callback);
        output_->StartNewRecord(HPROF_TAG_HEAP_DUMP_END, 0x0);
        output_->EndRecord();
    }

    void ProcessHeader(bool string_first) {
        WriteFixedHeader();

        if (string_first) {
            WriteStringTable();
        }
        WriteClassTable();
        WriteStackTraces();
        if (!string_first) {
            WriteStringTable();
        }
        output_->EndRecord();
    }

    void WriteFixedHeader() {
        char magic[] = "JAVA PROFILE 1.0.3";
        __ AddU1List(reinterpret_cast<uint8_t*>(magic), sizeof(magic));
        __ AddU4(sizeof(uint32_t));
        __ AddU4(0x0);
        __ AddU4(0x0);
    }

    void WriteStringTable() {
        for (const auto& p : strings_) {
            const std::string& string = p.first;
            const HprofStringId id = p.second;

            output_->StartNewRecord(HPROF_TAG_STRING, kHprofTime);

            __ AddU4(id);
            __ AddUtf8String(const_cast<char *>(string.c_str()));
        }
    }

    void WriteClassTable() {
        for (const auto& p : classes_) {
            mirror::Class c = p.first;
            HprofClassSerialNumber sn = p.second;
            output_->StartNewRecord(HPROF_TAG_LOAD_CLASS, kHprofTime);

            __ AddU4(sn);
            __ AddObjectId(c);
            __ AddStackTraceSerialNumber(kHprofNullStackTrace);
            __ AddStringId(LookupClassNameId(c));
        }
    }

    void WriteStackTraces() {
        output_->StartNewRecord(HPROF_TAG_STACK_TRACE, kHprofTime);
        __ AddStackTraceSerialNumber(kHprofNullStackTrace);
        __ AddU4(kHprofNullThread);
        __ AddU4(0);
    }

    HprofStringId LookupStringId(const std::string& string) {
        auto it = strings_.find(string);
        if (it != strings_.end()) {
            return it->second;
        }

        HprofStringId id = next_string_id_++;
        strings_.insert(std::pair<std::string, HprofStringId>(string, id));
        return id;
    }

    HprofStringId LookupClassNameId(mirror::Class& c) {
        std::string desc = c.PrettyDescriptor();
        return LookupStringId(desc);
    }

    HprofClassObjectId LookupClassId(mirror::Class& c) {
        if (c.Ptr()) {
            auto it = classes_.find(c.Ptr());
            if (it == classes_.end()) {
                // first time to see this class
                HprofClassSerialNumber sn = next_class_serial_number_++;
                classes_.insert(std::pair<uint32_t, HprofClassSerialNumber>(c.Ptr(), sn));
                // Make sure that we've assigned a string ID for this class' name
                LookupClassNameId(c);
            }
        }
        return c.Ptr();
    }

    void StartNewHeapDumpSegment() {
        // This flushes the old segment and starts a new one.
        output_->StartNewRecord(HPROF_TAG_HEAP_DUMP_SEGMENT, kHprofTime);
        objects_in_segment_ = 0;
        // Starting a new HEAP_DUMP resets the heap to default.
        current_heap_ = HPROF_HEAP_DEFAULT;
    }

    void CheckHeapSegmentConstraints() {
        if (objects_in_segment_ >= kMaxObjectsPerSegment
                || output_->Length() >= kMaxBytesPerSegment) {
            StartNewHeapDumpSegment();
        }
    }

    const char* filename_;
    bool visible_;
    bool first_;

    EndianOutput* output_ = nullptr;
    HprofHeapId current_heap_ = HPROF_HEAP_DEFAULT;  // Which heap we're currently dumping.
    size_t objects_in_segment_ = 0;

    size_t total_objects_ = 0u;

    HprofStringId next_string_id_ = 0x400000;
    std::map<std::string, HprofStringId> strings_;

    HprofClassSerialNumber next_class_serial_number_ = 1;
    std::map<uint32_t, HprofClassSerialNumber> classes_;
};

bool Hprof::AddRuntimeInternalObjectsField(mirror::Class& klass) {
    if (klass.IsDexCacheClass())
        return true;

    mirror::Class super_clazz = klass.GetSuperClass();
    if (klass.IsClassLoaderClass() && super_clazz.Ptr()
            && super_clazz.IsObjectClass()) {
        return true;
    }
    return false;
}

bool Hprof::DumpHeapObject(mirror::Object& object) {
    if (object.IsClass()) {
        mirror::Class thiz = object;
        if (thiz.IsRetired())
            return false;
    }

    ++total_objects_;

    Runtime& runtime = Runtime::Current();
    gc::Heap& heap = runtime.GetHeap();
    HprofHeapId heap_type = HPROF_HEAP_APP;
    gc::space::ContinuousSpace* space = heap.FindContinuousSpaceFromObject(object);
    if (space) {
        if (space->IsZygoteSpace()) {
            heap_type = HPROF_HEAP_ZYGOTE;
        } else if (space->IsImageSpace()) {
            heap_type = HPROF_HEAP_IMAGE;
        }
    }
    CheckHeapSegmentConstraints();

    if (heap_type != current_heap_) {
        HprofStringId nameId;

        // This object is in a different heap than the current one.
        // Emit a HEAP_DUMP_INFO tag to change heaps.
        __ AddU1(HPROF_HEAP_DUMP_INFO);
        __ AddU4(static_cast<uint32_t>(heap_type));

        switch (heap_type) {
            case HPROF_HEAP_APP:
                nameId = LookupStringId("app");
                break;
            case HPROF_HEAP_ZYGOTE:
                nameId = LookupStringId("zygote");
                break;
            case HPROF_HEAP_IMAGE:
                nameId = LookupStringId("image");
                break;
            default:
                nameId = LookupStringId("<ILLEGAL>");
                break;
        }

        __ AddStringId(nameId);
        current_heap_ = heap_type;
    }

    mirror::Class klass = object.GetClass();
    if (klass.Ptr()) {
        if (object.IsClass()) {
            mirror::Class thiz = object;
            DumpHeapClass(thiz);
        } else if (klass.IsArrayClass()) {
            mirror::Array thiz = object;
            DumpHeapArray(thiz, klass);
        } else {
            DumpHeapInstanceObject(object, klass);
        }
    }
    ++objects_in_segment_;
    return false;
}

void Hprof::DumpHeapClass(mirror::Class& klass) {
    if (!klass.IsResolved())
        return;

    HLOGV("%s 0x%lx\n", __func__, klass.Ptr());
    HLOGV("%s\n", klass.PrettyDescriptor().c_str());

    uint64_t num_static_fields = klass.NumStaticFields();
    uint64_t total_class_size = klass.GetClassSize();
    uint64_t base_class_size = SIZEOF(Class);

    uint64_t base_overhead_size = total_class_size - base_class_size;
    uint64_t class_static_fields_size = 0;

    auto cloc_class_static_fields_size = [&](art::ArtField& field) -> bool {
        uint64_t size = 0;
        Android::SignatureToBasicTypeAndSize(field.GetTypeDescriptor(), &size, "B");
        class_static_fields_size += size;
        return false;
    };
    Android::ForeachStaticField(klass, cloc_class_static_fields_size);

    uint64_t base_no_statics_overhead_size = base_overhead_size - class_static_fields_size;
    uint64_t java_heap_overhead_size = base_no_statics_overhead_size;

    if (java_heap_overhead_size > 4) {
        __ AddU1(HPROF_PRIMITIVE_ARRAY_DUMP);
        __ AddClassStaticsId(klass);
        __ AddStackTraceSerialNumber(kHprofNullStackTrace);
        __ AddU4(java_heap_overhead_size - 4);
        __ AddU1(Android::basic_byte);
        for (size_t i = 0; i < java_heap_overhead_size - 4; ++i) {
            output_->AddU1(0);
        }
    }

    uint64_t java_heap_overhead_field_count = java_heap_overhead_size > 0
                                                ? (java_heap_overhead_size == 3 ? 2u : 1u)
                                                : 0;

    mirror::Class super_class = klass.GetSuperClass();
    mirror::Class class_loader = klass.GetClassLoader();

    __ AddU1(HPROF_CLASS_DUMP);
    __ AddClassId(LookupClassId(klass));
    __ AddStackTraceSerialNumber(kHprofNullStackTrace);
    __ AddClassId(LookupClassId(super_class));
    __ AddObjectId(class_loader);
    __ AddObjectId(HprofInvalidObject);    // no signer
    __ AddObjectId(HprofInvalidObject);    // no prot domain
    __ AddObjectId(HprofInvalidObject);    // reserved
    __ AddObjectId(HprofInvalidObject);    // reserved

    // Instance size.
    if (klass.IsClassClass()) {
        __ AddU4(0);
    } else if (klass.IsStringClass()) {
        __ AddU4(SIZEOF(String));
    } else if (klass.IsArrayClass() || klass.IsPrimitive()) {
        __ AddU4(0);
    } else {
        __ AddU4(klass.GetObjectSize());  // instance size
    }

    __ AddU2(0);  // empty const pool

    mirror::Class class_class = klass.GetClass();
    mirror::Class super_class_class = class_class.GetSuperClass();

    uint64_t static_fields_reported = class_class.NumInstanceFields()
                                    + super_class_class.NumInstanceFields()
                                    + java_heap_overhead_field_count
                                    + num_static_fields;
    __ AddU2(static_cast<uint16_t>(static_fields_reported));

    if (java_heap_overhead_size != 0) {
        __ AddStringId(LookupStringId(kClassOverheadName));
        uint64_t overhead_fields = 0;
        if (java_heap_overhead_size > 4) {
            __ AddU1(Android::basic_object);
            __ AddClassStaticsId(klass);
            ++overhead_fields;
        } else {
            switch (java_heap_overhead_size) {
                case 4: {
                    __ AddU1(Android::basic_int);
                    __ AddU4(0);
                    ++overhead_fields;
                } break;
                case 2: {
                    __ AddU1(Android::basic_short);
                    __ AddU2(0);
                    ++overhead_fields;
                } break;
                case 3: {
                    __ AddU1(Android::basic_short);
                    __ AddU2(0);
                    __ AddStringId(LookupStringId(std::string(kClassOverheadName) + "2"));
                    ++overhead_fields;
                }
                [[fallthrough]];
                case 1: {
                    __ AddU1(Android::basic_byte);
                    __ AddU1(0);
                    ++overhead_fields;
                } break;
            }
        }
    }

    auto static_field_writer = [&](ArtField& field, auto name_fn) -> bool {
        __ AddStringId(LookupStringId(name_fn(field)));

        uint64_t size = 0;
        Android::BasicType type = Android::SignatureToBasicTypeAndSize(field.GetTypeDescriptor(), &size, "B");
        __ AddU1(type);
        switch (type) {
            case Android::basic_byte:
                __ AddU1(field.GetByte(klass));
                break;
            case Android::basic_boolean:
                __ AddU1(field.GetBoolean(klass));
                break;
            case Android::basic_char:
                __ AddU2(field.GetChar(klass));
                break;
            case Android::basic_short:
                __ AddU2(field.GetShort(klass));
                break;
            case Android::basic_float:
            case Android::basic_int:
            case Android::basic_object:
                __ AddU4(field.Get32(klass));
                break;
            case Android::basic_double:
            case Android::basic_long:
                __ AddU8(field.Get64(klass));
                break;
        }
        return false;
    };

    {
        auto class_instance_field_name_fn = [](ArtField& field) {
            return std::string("$class$") + field.GetName();
        };

        auto instance_field_writer_inner = [&](art::ArtField& field) -> bool {
            return static_field_writer(field, class_instance_field_name_fn);
        };
        Android::ForeachInstanceField(class_class, instance_field_writer_inner);
        Android::ForeachInstanceField(super_class_class, instance_field_writer_inner);
    }

    {
        auto class_static_field_name_fn = [](ArtField& field) {
            return field.GetName();
        };

        auto static_field_writer_inner = [&](art::ArtField& field) -> bool {
            return static_field_writer(field, class_static_field_name_fn);
        };
        Android::ForeachStaticField(klass, static_field_writer_inner);
    }

    int iFieldCount = klass.NumInstanceFields();
    bool add_internal_runtime_objects = AddRuntimeInternalObjectsField(klass);
    if (klass.IsStringClass() || add_internal_runtime_objects) {
        __ AddU2((uint16_t)iFieldCount + 1);
    } else {
        __ AddU2((uint16_t)iFieldCount);
    }

    auto klass_field_writer_inner = [&](art::ArtField& field) -> bool {
        __ AddStringId(LookupStringId(field.GetName()));
        Android::BasicType type = Android::SignatureToBasicTypeAndSize(field.GetTypeDescriptor(), nullptr, "B");
        __ AddU1(type);
        return false;
    };
    Android::ForeachInstanceField(klass, klass_field_writer_inner);

    // Add native value character array for strings / byte array for compressed strings.
    if (klass.IsStringClass()) {
        __ AddStringId(LookupStringId("value"));
        __ AddU1(Android::basic_object);
    } else if (add_internal_runtime_objects) {
        __ AddStringId(LookupStringId("runtimeInternalObjects"));
        __ AddU1(Android::basic_object);
    }
}

void Hprof::DumpHeapArray(mirror::Array& array, mirror::Class& klass) {
    HLOGV("%s 0x%lx\n", __func__, array.Ptr());
    HLOGV("%s\n", klass.PrettyDescriptor().c_str());

    uint32_t length = array.GetLength();
    if (array.IsObjectArray()) {
        // obj is an object array.
        __ AddU1(HPROF_OBJECT_ARRAY_DUMP);

        __ AddObjectId(array);
        __ AddStackTraceSerialNumber(kHprofNullStackTrace);
        __ AddU4(length);
        __ AddClassId(LookupClassId(klass));

        // Dump the elements, which are always objects or null.
        __ AddIdList(array);
    } else {
        uint64_t size;
        mirror::Class component = klass.GetComponentType();
        Android::BasicType type = Android::SignatureToBasicTypeAndSize(Primitive::Descriptor(component.GetPrimitiveType()), &size, "B");

        // obj is a primitive array.
        __ AddU1(HPROF_PRIMITIVE_ARRAY_DUMP);

        __ AddObjectId(array);
        __ AddStackTraceSerialNumber(kHprofNullStackTrace);
        __ AddU4(length);
        __ AddU1(type);

        // Dump the raw, packed element values.
        if (size == 1) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint8_t), 0), array);
            __ AddU1List(reinterpret_cast<uint8_t*>(ref.Real()), length);
        } else if (size == 2) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint16_t), 0), array);
            __ AddU2List(reinterpret_cast<uint16_t*>(ref.Real()), length);
        } else if (size == 4) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), 0), array);
            __ AddU4List(reinterpret_cast<uint32_t*>(ref.Real()), length);
        } else if (size == 8) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint64_t), 0), array);
            __ AddU8List(reinterpret_cast<uint64_t*>(ref.Real()), length);
        }
    }
}

void Hprof::DumpHeapInstanceObject(mirror::Object& object, mirror::Class& klass) {
    HLOGV("%s 0x%lx\n", __func__, object.Ptr());
    HLOGV("%s\n", klass.PrettyDescriptor().c_str());

    __ AddU1(HPROF_INSTANCE_DUMP);
    __ AddObjectId(object);
    __ AddStackTraceSerialNumber(kHprofNullStackTrace);
    __ AddClassId(LookupClassId(klass));

    uint64_t size_patch_offset = output_->Length();
    __ AddU4(0x77777777);

    mirror::Object string_value = 0x0;
    mirror::Object fake_object_array = 0x0;

    mirror::Class super = klass;
    do {
        auto instance_field_writer = [&](ArtField& field) -> bool {
            uint64_t size;
            Android::BasicType type = Android::SignatureToBasicTypeAndSize(field.GetTypeDescriptor(), &size, "B");
            switch (type) {
                case Android::basic_byte:
                    __ AddU1(field.GetByte(object));
                    break;
                case Android::basic_boolean:
                    __ AddU1(field.GetBoolean(object));
                    break;
                case Android::basic_char:
                    __ AddU2(field.GetChar(object));
                    break;
                case Android::basic_short:
                    __ AddU2(field.GetShort(object));
                    break;
                case Android::basic_int:
                    if (field.offset() == OFFSET(String, count_) && super.IsStringClass()) {
                        mirror::String str = object;
                        __ AddU4(str.GetLength());
                        break;
                    }
                [[fallthrough]];
                case Android::basic_float:
                case Android::basic_object:
                    __ AddU4(field.Get32(object));
                    break;
                case Android::basic_double:
                case Android::basic_long:
                    __ AddU8(field.Get64(object));
                    break;
            }
            return false;
        };
        Android::ForeachInstanceField(super, instance_field_writer);

        if (super.IsStringClass()) {
            mirror::String str = object;
            if (str.GetLength() == 0) {
                string_value = str.Ptr() + kObjectAlignment;
            } else {
                string_value = str.Ptr() + SIZEOF(String);
            }
            __ AddObjectId(string_value);
        } else if (AddRuntimeInternalObjectsField(super)) {
            //fake_object_array = object.Ptr() + (kObjectAlignment / 2);
            //__ AddObjectId(fake_object_array);
        }
        super = super.GetSuperClass();
    } while (super.Ptr());

    __ UpdateU4(size_patch_offset, output_->Length() - (size_patch_offset + 4));

    if (string_value.Ptr()) {
        art::mirror::String str = object;
        __ AddU1(HPROF_PRIMITIVE_ARRAY_DUMP);
        __ AddObjectId(string_value);
        __ AddStackTraceSerialNumber(kHprofNullStackTrace);
        __ AddU4(str.GetLength());
        if (str.IsCompressed()) {
            __ AddU1(Android::basic_byte);
            __ AddU1List(str.GetValueCompressed(), str.GetLength());
        } else {
            __ AddU1(Android::basic_char);
            __ AddU2List(str.GetValue(), str.GetLength());
        }
    } else if (fake_object_array.Ptr()) {
        DumpFakeObjectArray(fake_object_array);
    }
}

void Hprof::DumpFakeObjectArray(mirror::Object& object) {
    __ AddU1(HPROF_OBJECT_ARRAY_DUMP);
    __ AddObjectId(object);
    __ AddStackTraceSerialNumber(kHprofNullStackTrace);
    __ AddU4(0);
    __ AddClassId(0);
}

void DumpHeap(const char* output, bool visible) {
    Hprof hprof(output, visible);
    hprof.Dump();
}

} // namespace hprof
} // namespace art
