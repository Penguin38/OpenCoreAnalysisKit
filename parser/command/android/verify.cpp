/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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

#include "logger/log.h"
#include "android.h"
#include "command/android/verify.h"
#include "command/android/cmd_print.h"
#include <vector>

void JavaVerify::init(int opt) {
    options = opt;
}

void JavaVerify::verify(art::mirror::Object& object) {
    art::mirror::Class clazz = object.GetClass();

    if (options & CHECK_FULL_BAD_OBJECT) {
        if (object.IsClass()) {
            art::mirror::Class thiz = object;
            VerifyClassObject(thiz);
        } else if (clazz.IsArrayClass()) {
            art::mirror::Array thiz = object;
            VerifyArrayObject(thiz);
        } else {
            VerifyInstanceObject(object);
        }
    }

    if (options & CHECK_FULL_CONFLICT_METHOD) {
        if (object.IsClass()) {

        }
    }

    if (options & CHECK_FULL_REUSE_DEX_PC_PTR) {
        if (object.IsClass()) {

        }
    }
}

void JavaVerify::VerifyClassObject(art::mirror::Class& clazz) {
    art::mirror::Class current = clazz;
    std::string format = PrintCommand::FormatSize(clazz.SizeOf());
    PrintCommand::Options options;
    bool need_header = true;

    std::vector<art::ArtField> fields;
    auto callback = [&](art::ArtField& field) -> bool {
        fields.push_back(field);
        return false;
    };

    Android::ForeachStaticField(current, callback);
    current = clazz.GetClass();
    if (current.Ptr() && current.IsValid()) {
        Android::ForeachInstanceField(current, callback);
        current = current.GetSuperClass();

        if (current.Ptr() && current.IsValid())
            Android::ForeachInstanceField(current, callback);
    }
    std::sort(fields.begin(), fields.end(), art::ArtField::Compare);

    for (auto& field : fields) {
        uint64_t size;
        const char* sig = field.GetTypeDescriptor();
        Android::BasicType type = Android::SignatureToBasicTypeAndSize(sig, &size, "B");
        if (type != Android::basic_object)
            continue;

        art::mirror::Object tmp(field.GetObj(clazz), clazz);
        if (tmp.Ptr() && !tmp.IsValid()) {
            if (need_header) {
                LOGE("verify class: " ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, clazz.Ptr());
                LOGI("Class Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());
                need_header = false;
            }
            PrintCommand::PrintField(format.c_str(), current, clazz, field, options);
        }
    }
    if (!need_header) ENTER();
}

void JavaVerify::VerifyArrayObject(art::mirror::Array& array) {
    art::mirror::Class clazz = array.GetClass();
    uint32_t length = array.GetLength();
    bool need_header = true;

    if (array.IsObjectArray()) {
        for (int32_t i = 0; i < length; ++i) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), i), array);
            art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
            if (object.Ptr() && !object.IsValid()) {
                if (need_header) {
                    LOGE("verify array: " ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, array.Ptr());
                    LOGI("Array Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());
                    need_header = false;
                }
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, i, object.Ptr());
            }
        }
    }
    if (!need_header) ENTER();
}

void JavaVerify::VerifyInstanceObject(art::mirror::Object& object) {
    art::mirror::Class clazz = object.GetClass();
    art::mirror::Class super = clazz;
    std::string format = PrintCommand::FormatSize(object.SizeOf());
    PrintCommand::Options options;
    bool need_header = true;
    do {
        std::vector<art::ArtField> fields;
        auto callback = [&](art::ArtField& field) -> bool {
            fields.push_back(field);
            return false;
        };
        Android::ForeachInstanceField(super, callback);
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);

        for (auto& field : fields) {
            uint64_t size;
            const char* sig = field.GetTypeDescriptor();
            Android::BasicType type = Android::SignatureToBasicTypeAndSize(sig, &size, "B");
            if (type != Android::basic_object)
                continue;

            art::mirror::Object tmp(field.GetObj(object), object);
            if (tmp.Ptr() && !tmp.IsValid()) {
                if (need_header) {
                    LOGE("verify instance: " ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, object.Ptr());
                    LOGI("Object Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());
                    need_header = false;
                }
                PrintCommand::PrintField(format.c_str(), super, object, field, options);
            }
        }

        super = super.GetSuperClass();
    } while (super.Ptr());
    if (!need_header) ENTER();
}
