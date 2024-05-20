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
#include "command/cmd_class.h"
#include "command/cmd_print.h"
#include "dex/modifiers.h"
#include "android.h"
#include "runtime/mirror/iftable.h"
#include "api/core.h"
#include <stdio.h>

int ClassCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    dump_all = !(argc > 1)? true : false;
    total_classes = 0;
    auto callback = [&](art::mirror::Object& object) -> bool {
        return PrintClass(object, argv[1]);
    };
    Android::ForeachObjects(callback);
    return 0;
}

bool ClassCommand::PrintClass(art::mirror::Object& object, const char* classname) {
    if (!object.IsClass())
        return false;

    art::mirror::Class thiz = object;
    if (dump_all) {
        total_classes++;
        LOGI("[%ld] 0x%lx %s\n", total_classes, thiz.Ptr(), thiz.PrettyDescriptor().c_str());
    } else if (thiz.PrettyDescriptor() == classname) {
        PrintPrettyClassContent(thiz);
    }

    return false;
}

void ClassCommand::PrintPrettyClassContent(art::mirror::Class& clazz) {
    LOGI("[0x%lx]\n", clazz.Ptr());
    art::mirror::Class super = clazz.GetSuperClass();
    art::mirror::IfTable& iftable = clazz.GetIfTable();
    int32_t ifcount = iftable.Count();
    bool needEnd = false;
    if (super.Ptr()) {
        LOGI("%sclass %s extends %s {\n",
                art::PrettyJavaAccessFlags(clazz.GetAccessFlags()).c_str(),
                clazz.PrettyDescriptor().c_str(), super.PrettyDescriptor().c_str());
    } else {
        LOGI("%sclass %s {\n",
                art::PrettyJavaAccessFlags(clazz.GetAccessFlags()).c_str(),
                clazz.PrettyDescriptor().c_str());
    }
    if (ifcount > 0) {
        LOGI("  // Implements:\n");
        for (int i = 0; i < ifcount; ++i) {
            art::mirror::Class interface = iftable.GetInterface(i);
            LOGI("    %s\n", interface.PrettyDescriptor().c_str());
        }
        needEnd = true;
    }
    std::string format = PrintCommand::FormatSize(clazz.SizeOf());
    std::string format_nonenter = format;
    format.append("\n");

    art::mirror::Class current = clazz;
    if (clazz.NumStaticFields()) {
        if (needEnd) LOGI("\n");
        LOGI("  // Class static fields:\n");
        needEnd = true;
    }
    auto print_static_field = [&](art::ArtField& field) -> bool {
        PrintCommand::PrintField(format_nonenter.c_str(), current, clazz, field);
        return false;
    };
    Android::ForeachStaticField(current, print_static_field);

    if (clazz.NumInstanceFields()) {
        if (needEnd) LOGI("\n");
        LOGI("  // Object instance fields:\n");
        needEnd = true;
    }
    super = clazz;
    do {
        if (clazz != super) {
            if (needEnd) LOGI("\n");
            LOGI("  // extends %s\n", super.PrettyDescriptor().c_str());
            needEnd = true;
        }

        auto callback = [&](art::ArtField& field) -> bool {
            LOGI(format.c_str(), field.offset(), art::PrettyJavaAccessFlags(field.access_flags()).c_str(),
                                 field.PrettyTypeDescriptor().c_str(), field.GetName());
            return false;
        };
        Android::ForeachInstanceField(super, callback);

        super = super.GetSuperClass();
    } while (super.Ptr());

    if (clazz.NumMethods()) {
        if (needEnd) LOGI("\n");
        LOGI("  // Methods:\n");
        needEnd = true;
    }
    auto print_method = [](art::ArtMethod& method) -> bool {
        LOGI("    [0x%lx] %s%s\n", method.Ptr(), art::PrettyJavaAccessFlags(method.access_flags()).c_str(),
                                 method.PrettyMethod().c_str());
        return false;
    };
    Android::ForeachArtMethods(current, print_method);

    LOGI("}\n");
}

void ClassCommand::usage() {
    LOGI("Usage: class [CLASSNAME]\n");
}
