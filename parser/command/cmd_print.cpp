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

#include "logger/log.h"
#include "android.h"
#include "common/bit.h"
#include "command/cmd_print.h"
#include "command/cmd_format_dump.h"
#include "command/command_manager.h"
#include "common/exception.h"
#include "base/utils.h"
#include "api/core.h"
#include "runtime/runtime_globals.h"
#include "dex/modifiers.h"
#include "dex/primitive.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <iomanip>
#include <vector>

bool PrintCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return false;

    reference = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"binary",  no_argument,       0,  'b'},
        {"ref",     required_argument, 0,  'r'},
        {"format",  no_argument,       0,  'f'},
        {"hex",     no_argument,       0,  'x'},
        {0,         0,                 0,   0 }
    };

    while ((opt = getopt_long(argc, argv, "bfxr:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'r':
                reference = true;
                break;
        }
    }

    if (reference) Android::Prepare();
    return true;
}

int PrintCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return 0;

    binary = false;
    reference = false;
    format_dump = false;
    format_hex = false;
    deep = 1;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"binary",  no_argument,       0,  'b'},
        {"ref",     required_argument, 0,  'r'},
        {"format",  no_argument,       0,  'f'},
        {"hex",     no_argument,       0,  'x'},
        {0,         0,                 0,   0 }
    };
    
    while ((opt = getopt_long(argc, argv, "bfxr:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'b':
                binary = true;
                break;
            case 'r':
                reference = true;
                deep = std::atoi(optarg);
                break;
            case 'f':
                format_dump = true;
                break;
            case 'x':
                format_hex = true;
                break;
        }
    }

    if (optind >= argc) {
        usage();
        return 0;
    }

    art::mirror::Object ref = Utils::atol(argv[optind]);
    FormatDumpCall format_call = nullptr;
    if (format_dump) format_call = GetFormatDumpCall(ref);
    if (format_call) {
        format_call("", ref);
    } else {
        DumpObject(ref);
        if (binary) {
            LOGI(ANSI_COLOR_LIGHTRED "Binary:\n" ANSI_COLOR_RESET);
            uint64_t real_size = RoundUp(ref.SizeOf(), art::kObjectAlignment);
            int argc = 4;
            std::string bs = Utils::ToHex(ref.Ptr());
            std::string es = Utils::ToHex(ref.Ptr() + real_size);
            char* argv[4] = {
                const_cast<char*>("rd"),
                const_cast<char*>(bs.c_str()),
                const_cast<char*>("-e"),
                const_cast<char*>(es.c_str())};
            CommandManager::Execute(argv[0], argc, argv);
        }
    }
    return 0;
}

void PrintCommand::DumpObject(art::mirror::Object& object) {
    uint64_t size = object.SizeOf();
    uint64_t real_size = RoundUp(size, art::kObjectAlignment);
    LOGI("Size: 0x%lx\n", real_size);
    if (size != real_size) {
        LOGI("Padding: 0x%lx\n", real_size - size);
    }

    art::mirror::Class clazz = object.GetClass();
    if (clazz.Ptr()) {
        try {
            if (object.IsClass()) {
                art::mirror::Class thiz = object;
                DumpClass(thiz);
            } else if (clazz.IsArrayClass()) {
                art::mirror::Array thiz = object;
                DumpArray(thiz);
            } else {
                DumpInstance(object);
            }
        } catch(InvalidAddressException e) {
            LOGE("%s\n", e.what());
        }

        if (reference) {
            LOGI(ANSI_COLOR_LIGHTRED "Reference:\n" ANSI_COLOR_RESET);
            int cur_deep = 0;
            auto callback = [&](art::mirror::Object& reference) -> bool {
                return PrintReference(object, reference, cur_deep);
            };
            Android::ForeachObjects(callback);
        }
    }
}

bool PrintCommand::PrintReference(art::mirror::Object& object, art::mirror::Object& reference, int cur_deep) {
    if (cur_deep >= deep)
        return true;

    uint64_t real_size = RoundUp(reference.SizeOf(), art::kObjectAlignment);
    uint64_t count = real_size / sizeof(uint32_t);
    std::string prefix;
    for (int cur = -1; cur < cur_deep; ++cur) {
        prefix.append("  ");
    }

    for (int pos = 0; pos < count; ++pos) {
        if (object.Ptr() == reference.value32Of(pos * sizeof(uint32_t))) {
            art::mirror::Class ref_thiz = 0x0;
            if (reference.IsClass()) {
                ref_thiz = reference;
            } else {
                ref_thiz = reference.GetClass();
            }
            LOGI("%s--> " ANSI_COLOR_LIGHTYELLOW "0x%lx " ANSI_COLOR_LIGHTCYAN "%s\n" ANSI_COLOR_RESET,
                    prefix.c_str(), reference.Ptr(), ref_thiz.PrettyDescriptor().c_str());
            if (cur_deep + 1 < deep) {
                auto callback = [&](art::mirror::Object& second) -> bool {
                    return PrintReference(reference, second, cur_deep + 1);
                };
                Android::ForeachObjects(callback);
            }
            break;
        }
    }
    return false;;
}

void PrintCommand::DumpClass(art::mirror::Class& clazz) {
    LOGI("Class Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());

    std::string format = FormatSize(clazz.SizeOf());
    art::mirror::Class current = clazz;

    std::vector<art::ArtField> fields;
    auto callback = [&](art::ArtField& field) -> bool {
        fields.push_back(field);
        return false;
    };

    Android::ForeachStaticField(current, callback);
    if (current.NumStaticFields()) {
        LOGI(ANSI_COLOR_LIGHTCYAN "  // info %s\n" ANSI_COLOR_RESET, current.PrettyDescriptor().c_str());
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
        for (auto& field : fields) {
            PrintCommand::PrintField(format.c_str(), current, clazz, field, format_hex);
        }
    }
    fields.clear();

    current = clazz.GetClass();
    LOGI(ANSI_COLOR_LIGHTCYAN "  // info %s\n" ANSI_COLOR_RESET, current.PrettyDescriptor().c_str());
    Android::ForeachInstanceField(current, callback);
    std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
    for (auto& field : fields) {
        PrintCommand::PrintField(format.c_str(), current, clazz, field, format_hex);
    }
    fields.clear();

    current = current.GetSuperClass();
    LOGI(ANSI_COLOR_LIGHTCYAN "  // extends %s\n" ANSI_COLOR_RESET, current.PrettyDescriptor().c_str());
    Android::ForeachInstanceField(current, callback);
    std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
    for (auto& field : fields) {
        PrintCommand::PrintField(format.c_str(), current, clazz, field, format_hex);
    }
}

void PrintCommand::DumpArray(art::mirror::Array& array) {
    art::mirror::Class clazz = array.GetClass();
    LOGI("Array Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());

    uint32_t length = array.GetLength();
    if (array.IsObjectArray()) {
        for (int32_t i = 0; i < length; ++i) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), i), array);
            art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
            if (object.IsValid() && object.IsString()) {
                art::mirror::String str = object;
                LOGI("    [%d] \"" ANSI_COLOR_LIGHTMAGENTA "%s" ANSI_COLOR_RESET "\"\n", i, str.ToModifiedUtf8().c_str());
            } else {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, i, object.Ptr());
            }
        }
    } else {
        uint64_t size;
        art::mirror::Class component = clazz.GetComponentType();
        Android::BasicType type = Android::SignatureToBasicTypeAndSize(art::Primitive::Descriptor(component.GetPrimitiveType()), &size);
        for (int32_t i = 0; i < length; ++i) {
            switch (size) {
                case 1: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint8_t), i), array);
                    PrintArrayElement(i, type, ref, format_hex);
                } break;
                case 2: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint16_t), i), array);
                    PrintArrayElement(i, type, ref, format_hex);
                } break;
                case 4: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), i), array);
                    PrintArrayElement(i, type, ref, format_hex);
                } break;
                case 8: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint64_t), i), array);
                    PrintArrayElement(i, type, ref, format_hex);
                } break;
            }
        }
    }
}

void PrintCommand::DumpInstance(art::mirror::Object& object) {
    art::mirror::Class clazz = object.GetClass();
    LOGI("Object Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());

    std::string format = FormatSize(object.SizeOf());
    art::mirror::Class super = clazz;
    do {
        if (clazz != super) {
            LOGI(ANSI_COLOR_LIGHTCYAN "  // extends %s\n" ANSI_COLOR_RESET, super.PrettyDescriptor().c_str());
        }

        if (super.IsStringClass()) {
            art::mirror::String str = object;
            if (str.GetLength() != 0) {
                LOGI(format.c_str(), SIZEOF(String), "virutal ", "char[]", "values");
                LOGI(" = \"" ANSI_COLOR_LIGHTMAGENTA "%s" ANSI_COLOR_RESET "\"\n", str.ToModifiedUtf8().c_str());
            }
        }

        std::vector<art::ArtField> fields;
        auto callback = [&](art::ArtField& field) -> bool {
            fields.push_back(field);
            return false;
        };
        Android::ForeachInstanceField(super, callback);
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);

        for (auto& field : fields) {
            PrintCommand::PrintField(format.c_str(), super, object, field, format_hex);
        }

        super = super.GetSuperClass();
    } while (super.Ptr());
}

void PrintCommand::PrintField(const char* format, art::mirror::Class& clazz,
        art::mirror::Object& object, art::ArtField& field, bool format_hex) {
    uint64_t size;
    const char* sig = field.GetTypeDescriptor();
    Android::BasicType type = Android::SignatureToBasicTypeAndSize(sig, &size, "B");
    LOGI(format, field.offset(), art::PrettyJavaAccessFlags(field.access_flags()).c_str(), field.PrettyTypeDescriptor().c_str(), field.GetName());
    switch (type) {
        case Android::basic_byte:
            LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, field.GetByte(object));
            break;
        case Android::basic_boolean:
            LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, field.GetBoolean(object) ? "true" : "false");
            break;
        case Android::basic_char:
            LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, field.GetChar(object));
            break;
        case Android::basic_short:
            if (format_hex) {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, field.GetShort(object));
            } else {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, field.GetShort(object));
            }
            break;
        case Android::basic_int: {
            int32_t value = 0;
            if (field.offset() == OFFSET(String, count_)
                    && clazz.IsValid() && clazz.IsStringClass()) {
                art::mirror::String str = object;
                value = str.GetLength();
            } else {
                value = field.GetInt(object);
            }
            if (format_hex) {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, value);
            } else {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, value);
            }
        } break;
        case Android::basic_float:
            LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "%f\n" ANSI_COLOR_RESET, field.GetFloat(object));
            break;
        case Android::basic_object: {
            art::mirror::Object tmp(field.GetObj(object), object);
            if (tmp.IsValid() && tmp.IsString()) {
                art::mirror::String str = tmp;
                LOGI(" = \"" ANSI_COLOR_LIGHTMAGENTA "%s" ANSI_COLOR_RESET "\"\n", str.ToModifiedUtf8().c_str());
            } else {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, tmp.Ptr());
            }
        } break;
        case Android::basic_double:
            LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "%lf\n" ANSI_COLOR_RESET, field.GetDouble(object));
            break;
        case Android::basic_long:
            if (format_hex) {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, field.GetLong(object));
            } else {
                LOGI(" = " ANSI_COLOR_LIGHTMAGENTA "%ld\n" ANSI_COLOR_RESET, field.GetLong(object));
            }
            break;
    }
}

std::string PrintCommand::FormatSize(uint64_t size) {
    std::string format;
    format.append("    [0x%0");
    int num = 0;
    uint64_t current = size;
    do {
        current = current / 0xF;
        ++num;
    } while(current != 0);
    format.append(std::to_string(num));
    format.append("x] " ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_LIGHTRED "%s " ANSI_COLOR_RESET "%s");
    return format;
}

void PrintCommand::PrintArrayElement(uint32_t i, Android::BasicType type, api::MemoryRef& ref, bool format_hex) {
    switch (static_cast<uint32_t>(type)) {
        case Android::basic_byte:
            LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, i, *reinterpret_cast<uint8_t *>(ref.Real()));
            break;
        case Android::basic_boolean:
            LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, i, *reinterpret_cast<uint8_t *>(ref.Real()) ? "true" : "false");
            break;
        case Android::basic_char:
            LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, i, *reinterpret_cast<uint16_t *>(ref.Real()));
            break;
        case Android::basic_short:
            if (format_hex) {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, i, *reinterpret_cast<uint16_t *>(ref.Real()));
            } else {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, i, *reinterpret_cast<int16_t *>(ref.Real()));
            }
            break;
        case Android::basic_int:
            if (format_hex) {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%x\n" ANSI_COLOR_RESET, i, *reinterpret_cast<uint32_t *>(ref.Real()));
            } else {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, i, *reinterpret_cast<int32_t *>(ref.Real()));
            }
            break;
        case Android::basic_float:
            LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "%f\n" ANSI_COLOR_RESET, i, (float)*reinterpret_cast<uint32_t *>(ref.Real()));
            break;
        case Android::basic_double:
            LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "%lf\n" ANSI_COLOR_RESET, i, (double)*reinterpret_cast<uint64_t *>(ref.Real()));
            break;
        case Android::basic_long:
            if (format_hex) {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, i, *reinterpret_cast<uint64_t *>(ref.Real()));
            } else {
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "%ld\n" ANSI_COLOR_RESET, i, *reinterpret_cast<int64_t *>(ref.Real()));
            }
            break;
    }
}

void PrintCommand::usage() {
    LOGI("Usage: print|p <OBJECT> [OPTION..]\n");
    LOGI("Option:\n");
    LOGI("    -b, --binary       show object memory\n");
    LOGI("    -r, --ref <DEEP>   show object's ref\n");
    LOGI("    -f, --format       object format dump\n");
    LOGI("    -x, --hex          basic type hex print\n");
    ENTER();
    LOGI("core-parser> p 0x12c00000\n");
    LOGI("Size: 0x18\n");
    LOGI("Object Name: java.lang.ref.WeakReference\n");
    LOGI("  // extends java.lang.ref.Reference\n");
    LOGI("    [0x14] volatile java.lang.Object referent = 0x12c00018\n");
    LOGI("    [0x10] java.lang.ref.Reference queueNext = 0x0\n");
    LOGI("    [0x0c] final java.lang.ref.ReferenceQueue queue = 0x0\n");
    LOGI("    [0x08] java.lang.ref.Reference pendingNext = 0x0\n");
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x04] private transient int shadow$_monitor_ = 0\n");
    LOGI("    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f819828\n");
    ENTER();
    LOGI("core-parser> p 0x12c00000 -b -r 1 -x\n");
    LOGI("Size: 0x18\n");
    LOGI("Object Name: java.lang.ref.WeakReference\n");
    LOGI("  // extends java.lang.ref.Reference\n");
    LOGI("    [0x14] volatile java.lang.Object referent = 0x12c00018\n");
    LOGI("    [0x10] java.lang.ref.Reference queueNext = 0x0\n");
    LOGI("    [0x0c] final java.lang.ref.ReferenceQueue queue = 0x0\n");
    LOGI("    [0x08] java.lang.ref.Reference pendingNext = 0x0\n");
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x04] private transient int shadow$_monitor_ = 0x0\n");
    LOGI("    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f819828\n");
    LOGI("Reference:\n");
    LOGI("  --> 0x7010eac0 com.android.internal.os.BinderInternal\n");
    LOGI("Binary:\n");
    LOGI("12c00000: 000000006f819828  0000000000000000  (..o............\n");
    LOGI("12c00010: 12c0001800000000  00000000704662b0  .........bFp....\n");
}
