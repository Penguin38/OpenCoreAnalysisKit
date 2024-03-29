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
    art::mirror::Class super = clazz.GetSuperClass();
    art::mirror::IfTable& iftable = clazz.GetIfTable();
    int32_t ifcount = iftable.Count();
    LOGI("[0x%lx]\n%sclass %s extends %s %s\n", clazz.Ptr(),
            art::PrettyJavaAccessFlags(clazz.GetAccessFlags()).c_str(),
            clazz.PrettyDescriptor().c_str(), super.PrettyDescriptor().c_str(),
            ifcount > 0 ? "" : "{");
    if (ifcount > 0) {
        std::string ifdesc;
        for (int i = 0; i < ifcount; ++i) {
            art::mirror::Class interface = iftable.GetInterface(i);
            ifdesc.append(interface.PrettyDescriptor());
            if (i < ifcount - 1) ifdesc.append(", ");
        }
        LOGI("    implements %s {\n", ifdesc.c_str());
    }
    std::string format = PrintCommand::FormatSize(clazz.SizeOf());
    std::string format_nonenter = format;
    format.append("\n");

    art::mirror::Class current = clazz;
    if (clazz.NumStaticFields()) LOGI("  Class static fields:\n");
    auto print_static_field = [&](art::ArtField& field) -> bool {
        PrintCommand::PrintField(format_nonenter.c_str(), current, clazz, field);
        return false;
    };
    Android::ForeachStaticField(current, print_static_field);

    super = clazz;
    LOGI("  Object instance fields:\n");
    do {
        if (clazz != super) {
            LOGI("  extends %s\n", super.PrettyDescriptor().c_str());
        }

        auto callback = [&](art::ArtField& field) -> bool {
            LOGI(format.c_str(), field.offset(), art::PrettyJavaAccessFlags(field.access_flags()).c_str(),
                                 field.PrettyTypeDescriptor().c_str(), field.GetName());
            return false;
        };
        Android::ForeachInstanceField(super, callback);

        super = super.GetSuperClass();
    } while (super.Ptr());
    LOGI("}\n");
}

void ClassCommand::usage() {
    LOGI("Usage: class [CLASSNAME]\n");
}
