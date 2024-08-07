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
#include <unistd.h>
#include <getopt.h>
#include <vector>

int ClassCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    dump_all = !(argc > 1)? true : false;
    show_flag = 0;
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"method",  no_argument,       0,  'm'},
        {"static",  no_argument,       0,  's'},
        {"impl",    no_argument,       0,  'i'},
        {"field",  no_argument,        0,  'f'},
    };

    while ((opt = getopt_long(argc, argv, "msif",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                show_flag |= SHOW_METHOD;
                break;
            case 's':
                show_flag |= SHOW_STATIC;
                break;
            case 'i':
                show_flag |= SHOW_IMPL;
                break;
            case 'f':
                show_flag |= SHOW_FIELD;
                break;
        }
    }

    show_flag = !show_flag ? SHOW_ALL : show_flag;
    total_classes = 0;

    const char* classname = argv[optind];
    auto callback = [&](art::mirror::Object& object) -> bool {
        return PrintClass(object, classname);
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
        LOGI("[%ld] " ANSI_COLOR_LIGHTYELLOW "0x%lx" ANSI_COLOR_LIGHTCYAN " %s\n" ANSI_COLOR_RESET,
                total_classes, thiz.Ptr(), thiz.PrettyDescriptor().c_str());
    } else if (thiz.PrettyDescriptor() == classname) {
        PrintPrettyClassContent(thiz);
    }

    return false;
}

void ClassCommand::PrintPrettyClassContent(art::mirror::Class& clazz) {
    LOGI(ANSI_COLOR_LIGHTYELLOW "[0x%lx]\n" ANSI_COLOR_RESET, clazz.Ptr());
    art::mirror::Class super = clazz.GetSuperClass();
    art::mirror::IfTable& iftable = clazz.GetIfTable();
    int32_t ifcount = iftable.Count();
    bool needEnd = false;
    if (super.Ptr()) {
        LOGI(ANSI_COLOR_LIGHTGREEN "%s" "class " ANSI_COLOR_RESET "%s" ANSI_COLOR_LIGHTGREEN " extends " ANSI_COLOR_RESET "%s" " {\n",
                art::PrettyJavaAccessFlags(clazz.GetAccessFlags()).c_str(),
                clazz.PrettyDescriptor().c_str(), super.PrettyDescriptor().c_str());
    } else {
        LOGI(ANSI_COLOR_LIGHTGREEN "%s" "class " ANSI_COLOR_RESET "%s" " {\n",
                art::PrettyJavaAccessFlags(clazz.GetAccessFlags()).c_str(),
                clazz.PrettyDescriptor().c_str());
    }
    if (ifcount > 0 && (show_flag & SHOW_IMPL)) {
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Implements:\n" ANSI_COLOR_RESET);
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
    if (clazz.NumStaticFields() && (show_flag & SHOW_STATIC)) {
        if (needEnd) ENTER();
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Class static fields:\n" ANSI_COLOR_RESET);
        needEnd = true;
    }
    std::vector<art::ArtField> fields;
    auto print_static_field = [&](art::ArtField& field) -> bool {
        fields.push_back(field);
        return false;
    };
    if ((show_flag & SHOW_STATIC)) {
        Android::ForeachStaticField(current, print_static_field);
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
        for (auto& field : fields) {
            PrintCommand::PrintField(format_nonenter.c_str(), current, clazz, field);
        }
        fields.clear();
    }

    if (clazz.NumInstanceFields() && (show_flag & SHOW_FIELD)) {
        if (needEnd) ENTER();
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Object instance fields:\n" ANSI_COLOR_RESET);
        needEnd = true;
    }
    super = clazz;
    do {
        if (!(show_flag & SHOW_FIELD))
            break;

        if (clazz != super) {
            if (needEnd) ENTER();
            LOGI(ANSI_COLOR_LIGHTCYAN "  // extends %s\n" ANSI_COLOR_RESET, super.PrettyDescriptor().c_str());
            needEnd = true;
        }

        auto callback = [&](art::ArtField& field) -> bool {
            fields.push_back(field);
            return false;
        };
        Android::ForeachInstanceField(super, callback);
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
        for (auto& field : fields) {
            LOGI(format.c_str(), field.offset(), art::PrettyJavaAccessFlags(field.access_flags()).c_str(),
                                 field.PrettyTypeDescriptor().c_str(), field.GetName());
        }
        fields.clear();

        super = super.GetSuperClass();
    } while (super.Ptr());

    if (clazz.NumMethods() && (show_flag & SHOW_METHOD)) {
        if (needEnd) ENTER();
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Methods:\n" ANSI_COLOR_RESET);
        needEnd = true;
    }
    auto print_method = [](art::ArtMethod& method) -> bool {
        LOGI("    [0x%lx] " ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_RESET "%s\n",
                method.Ptr(), art::PrettyJavaAccessFlags(method.access_flags()).c_str(), method.ColorPrettyMethod().c_str());
        return false;
    };
    if ((show_flag & SHOW_METHOD)) Android::ForeachArtMethods(current, print_method);

    LOGI("}\n");
}

void ClassCommand::usage() {
    LOGI("Usage: class [CLASSNAME] [Option..]\n");
    LOGI("Option:\n");
    LOGI("    --method|-m: show class method\n");
    LOGI("    --impl|-i: show class implements class\n");
    LOGI("    --static|-s: show static field\n");
    LOGI("    --field|-f: show instance field\n");
}
