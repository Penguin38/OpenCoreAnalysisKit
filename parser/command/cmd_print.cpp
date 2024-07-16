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
        {0,           0,                 0,   0 }
    };

    while ((opt = getopt_long(argc, argv, "bfr:",
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
    deep = 1;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"binary",  no_argument,       0,  'b'},
        {"ref",     required_argument, 0,  'r'},
        {"format",  no_argument,       0,  'f'},
        {0,           0,                 0,   0 }
    };
    
    while ((opt = getopt_long(argc, argv, "bfr:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'b':
                binary = true;
                break;
            case 'r':
                reference = true;
                deep = atoi(optarg);
                break;
            case 'f':
                format_dump = true;
                break;
        }
    }

    art::mirror::Object ref = Utils::atol(argv[optind]);
    FormatDumpCall format_call = nullptr;
    if (format_dump) format_call = GetFormatDumpCall(ref);
    if (format_call) {
        format_call("", ref);
    } else {
        DumpObject(ref);
        if (binary) {
            LOGI("Binary:\n");
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
            LOGI("%s\n", e.what());
        }

        if (reference) {
            LOGI("Reference:\n");
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
            LOGI("%s--> 0x%lx %s\n", prefix.c_str(), reference.Ptr(), ref_thiz.PrettyDescriptor().c_str());
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
    LOGI("Class Name: %s\n", clazz.PrettyDescriptor().c_str());

    std::string format = FormatSize(clazz.SizeOf());
    art::mirror::Class current = clazz;

    std::vector<art::ArtField> fields;
    auto callback = [&](art::ArtField& field) -> bool {
        fields.push_back(field);
        return false;
    };

    Android::ForeachStaticField(current, callback);
    if (current.NumStaticFields()) {
        LOGI("  info %s\n", current.PrettyDescriptor().c_str());
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
        for (auto& field : fields) {
            PrintCommand::PrintField(format.c_str(), current, clazz, field);
        }
    }
    fields.clear();

    current = clazz.GetClass();
    LOGI("  info %s\n", current.PrettyDescriptor().c_str());
    Android::ForeachInstanceField(current, callback);
    std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
    for (auto& field : fields) {
        PrintCommand::PrintField(format.c_str(), current, clazz, field);
    }
    fields.clear();

    current = current.GetSuperClass();
    LOGI("  extends %s\n", current.PrettyDescriptor().c_str());
    Android::ForeachInstanceField(current, callback);
    std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
    for (auto& field : fields) {
        PrintCommand::PrintField(format.c_str(), current, clazz, field);
    }
}

void PrintCommand::DumpArray(art::mirror::Array& array) {
    art::mirror::Class clazz = array.GetClass();
    LOGI("Array Name: %s\n", clazz.PrettyDescriptor().c_str());

    uint32_t length = array.GetLength();
    if (array.IsObjectArray()) {
        for (int32_t i = 0; i < length; ++i) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), i), array);
            art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
            if (object.Ptr() && object.IsString()) {
                art::mirror::String str = object;
                LOGI("    [%d] \"%s\"\n", i, str.ToModifiedUtf8().c_str());
            } else {
                LOGI("    [%d] 0x%lx\n", i, object.Ptr());
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
                    PrintArrayElement(i, type, ref);
                } break;
                case 2: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint16_t), i), array);
                    PrintArrayElement(i, type, ref);
                } break;
                case 4: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), i), array);
                    PrintArrayElement(i, type, ref);
                } break;
                case 8: {
                    api::MemoryRef ref(array.GetRawData(sizeof(uint64_t), i), array);
                    PrintArrayElement(i, type, ref);
                } break;
            }
        }
    }
}

void PrintCommand::DumpInstance(art::mirror::Object& object) {
    art::mirror::Class clazz = object.GetClass();
    LOGI("Object Name: %s\n", clazz.PrettyDescriptor().c_str());

    std::string format = FormatSize(object.SizeOf());
    art::mirror::Class super = clazz;
    do {
        if (clazz != super) {
            LOGI("  extends %s\n", super.PrettyDescriptor().c_str());
        }

        if (super.IsStringClass()) {
            art::mirror::String str = object;
            if (str.GetLength() != 0) {
                LOGI(format.c_str(), SIZEOF(String), "virutal ", "char[]", "values");
                LOGI(" = \"%s\"\n", str.ToModifiedUtf8().c_str());
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
            PrintCommand::PrintField(format.c_str(), super, object, field);
        }

        super = super.GetSuperClass();
    } while (super.Ptr());
}

void PrintCommand::PrintField(const char* format, art::mirror::Class& clazz, art::mirror::Object& object, art::ArtField& field) {
    uint64_t size;
    const char* sig = field.GetTypeDescriptor();
    Android::BasicType type = Android::SignatureToBasicTypeAndSize(sig, &size, "B");
    LOGI(format, field.offset(), art::PrettyJavaAccessFlags(field.access_flags()).c_str(), field.PrettyTypeDescriptor().c_str(), field.GetName());
    switch (type) {
        case Android::basic_byte:
            LOGI(" = 0x%x\n", field.GetByte(object));
            break;
        case Android::basic_boolean:
            LOGI(" = %s\n", field.GetBoolean(object) ? "true" : "false");
            break;
        case Android::basic_char:
            LOGI(" = 0x%c\n", field.GetChar(object));
            break;
        case Android::basic_short:
            LOGI(" = 0x%x\n", field.GetShort(object));
            break;
        case Android::basic_int:
            if (field.offset() == OFFSET(String, count_) && clazz.IsStringClass()) {
                art::mirror::String str = object;
                LOGI(" = 0x%x\n", str.GetLength());
                break;
            } else {
                LOGI(" = 0x%x\n", field.GetInt(object));
            }
            break;
        case Android::basic_float:
            LOGI(" = 0x%f\n", field.GetFloat(object));
            break;
        case Android::basic_object: {
            art::mirror::Object tmp(field.GetObj(object), object);
            if (tmp.Ptr() && tmp.IsString()) {
                art::mirror::String str = tmp;
                LOGI(" = \"%s\"\n", str.ToModifiedUtf8().c_str());
            } else {
                LOGI(" = 0x%lx\n", tmp.Ptr());
            }
        } break;
        case Android::basic_double:
            LOGI(" = 0x%lf\n", field.GetDouble(object));
            break;
        case Android::basic_long:
            LOGI(" = 0x%lx\n", field.GetLong(object));
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
    format.append("x] %s%s %s");
    return format;
}

void PrintCommand::PrintArrayElement(uint32_t i, Android::BasicType type, api::MemoryRef& ref) {
    switch (static_cast<uint32_t>(type)) {
        case Android::basic_byte:
            LOGI("    [%d] 0x%x\n", i, *reinterpret_cast<uint8_t *>(ref.Real()));
            break;
        case Android::basic_boolean:
            LOGI("    [%d] %s\n", i, *reinterpret_cast<uint8_t *>(ref.Real()) ? "true" : "false");
            break;
        case Android::basic_char:
            LOGI("    [%d] 0x%x\n", i, *reinterpret_cast<uint16_t *>(ref.Real()));
            break;
        case Android::basic_short:
            LOGI("    [%d] 0x%x\n", i, *reinterpret_cast<uint16_t *>(ref.Real()));
            break;
        case Android::basic_int:
            LOGI("    [%d] 0x%x\n", i, *reinterpret_cast<uint32_t *>(ref.Real()));
            break;
        case Android::basic_float:
            LOGI("    [%d] %f\n", i, (double)*reinterpret_cast<uint32_t *>(ref.Real()));
            break;
        case Android::basic_double:
            LOGI("    [%d] %Lf\n", i, (long double)*reinterpret_cast<uint64_t *>(ref.Real()));
            break;
        case Android::basic_long:
            LOGI("    [%d] %lx\n", i, *reinterpret_cast<uint64_t *>(ref.Real()));
            break;
    }
}

void PrintCommand::usage() {
    LOGI("Usage: print|p <OBJECT> [option...]\n");
    LOGI("Option:\n");
    LOGI("    --binary|-b: show object memory.\n");
    LOGI("    --ref|-r <deep>: show object's ref.\n");
    LOGI("    --format|-f: format dump.\n");
}
