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

#include "logger/log.h"
#include "command/android/cmd_class.h"
#include "command/android/cmd_print.h"
#include "dex/modifiers.h"
#include "android.h"
#include "runtime/mirror/iftable.h"
#include "api/core.h"
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <vector>

int ClassCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return Command::FINISH;

    options.dump_all = true;
    options.show_flag = 0;
    options.format_hex = false;
    options.obj_each_flags = 0;
    options.total_classes = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"method",  no_argument,       0,  'm'},
        {"static",  no_argument,       0,  's'},
        {"impl",    no_argument,       0,  'i'},
        {"field",   no_argument,       0,  'f'},
        {"hex",     no_argument,       0,  'x'},
        {"app",     no_argument,       0,   1 },
        {"zygote",  no_argument,       0,   2 },
        {"image",   no_argument,       0,   3 },
        {"fake",    no_argument,       0,   4 },
        {0,         0,                 0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "msifx",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                options.show_flag |= SHOW_METHOD;
                break;
            case 's':
                options.show_flag |= SHOW_STATIC;
                break;
            case 'i':
                options.show_flag |= SHOW_IMPL;
                break;
            case 'f':
                options.show_flag |= SHOW_FIELD;
                break;
            case 'x':
                options.format_hex = true;
                break;
            case 1:
                options.obj_each_flags |= Android::EACH_APP_OBJECTS;
                break;
            case 2:
                options.obj_each_flags |= Android::EACH_ZYGOTE_OBJECTS;
                break;
            case 3:
                options.obj_each_flags |= Android::EACH_IMAGE_OBJECTS;
                break;
            case 4:
                options.obj_each_flags |= Android::EACH_FAKE_OBJECTS;
                break;
        }
    }
    options.optind = optind;

    if (!options.obj_each_flags) {
        options.obj_each_flags |= Android::EACH_APP_OBJECTS;
        options.obj_each_flags |= Android::EACH_ZYGOTE_OBJECTS;
        options.obj_each_flags |= Android::EACH_IMAGE_OBJECTS;
        options.obj_each_flags |= Android::EACH_FAKE_OBJECTS;
    }

    if (options.optind < argc) {
        options.dump_all = false;
        options.show_flag = !options.show_flag ? SHOW_ALL : options.show_flag;
    }

    Android::Prepare();
    return Command::ONCHLD;
}

int ClassCommand::main(int argc, char* const argv[]) {
    const char* classname = argv[options.optind];
    auto callback = [&](art::mirror::Object& object) -> bool {
        return PrintClass(object, classname);
    };

    try {
        if (!options.dump_all) {
            art::mirror::Object obj = Utils::atol(argv[options.optind]);
            if (obj.Ptr() && obj.IsValid() && obj.IsClass()) {
                art::mirror::Class thiz = obj;
                PrintPrettyClassContent(thiz);
            } else {
                Android::ForeachObjects(callback, options.obj_each_flags, false);
            }
        } else {
            Android::ForeachObjects(callback, options.obj_each_flags, false);
        }
    } catch(InvalidAddressException& e) {
        LOGW("The statistical process was interrupted!\n");
    }
    return 0;
}

bool ClassCommand::PrintClass(art::mirror::Object& object, const char* classname) {
    if (!object.IsClass())
        return false;

    art::mirror::Class thiz = object;
    if (options.dump_all) {
        options.total_classes++;
        LOGI("[%" PRId64 "] " ANSI_COLOR_LIGHTYELLOW "0x%" PRIx64 "" ANSI_COLOR_LIGHTCYAN " %s\n" ANSI_COLOR_RESET,
                options.total_classes, thiz.Ptr(), thiz.PrettyDescriptor().c_str());
        if (options.show_flag) PrintPrettyClassContent(thiz);
    } else if (thiz.PrettyDescriptor() == classname) {
        PrintPrettyClassContent(thiz);
    }

    return false;
}

void ClassCommand::PrintPrettyClassContent(art::mirror::Class& clazz) {
    LOGI(ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, clazz.Ptr());
    art::mirror::Class super = clazz.GetSuperClass();
    art::mirror::IfTable& iftable = clazz.GetIfTable();
    int32_t ifcount = (iftable.Ptr() && iftable.IsValid()) ? iftable.Count() : 0;
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
    if (ifcount > 0 && (options.show_flag & SHOW_IMPL)) {
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Implements:\n" ANSI_COLOR_RESET);
        for (int i = 0; i < ifcount; ++i) {
            art::mirror::Class interface = iftable.GetInterface(i);
            try {
                LOGI("    %s\n", interface.PrettyDescriptor().c_str());
            } catch(InvalidAddressException& e) {
                LOGI("    java.lang.Class<0x%" PRIx64 ">\n", interface.Ptr());
            }
        }
        needEnd = true;
    }
    std::string format = PrintCommand::FormatSize(clazz.SizeOf());
    std::string format_nonenter = format;
    format.append("\n");

    art::mirror::Class current = clazz;
    if (clazz.NumStaticFields() && (options.show_flag & SHOW_STATIC)) {
        if (needEnd) ENTER();
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Class static fields:\n" ANSI_COLOR_RESET);
        needEnd = true;
    }
    std::vector<art::ArtField> fields;
    auto print_static_field = [&](art::ArtField& field) -> bool {
        fields.push_back(field);
        return false;
    };
    if ((options.show_flag & SHOW_STATIC)) {
        Android::ForeachStaticField(current, print_static_field);
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);
        for (auto& field : fields) {
            PrintCommand::PrintField(format_nonenter.c_str(), current, clazz, field, options.format_hex);
        }
        fields.clear();
    }

    if (clazz.NumInstanceFields() && (options.show_flag & SHOW_FIELD)) {
        if (needEnd) ENTER();
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Object instance fields:\n" ANSI_COLOR_RESET);
        needEnd = true;
    }
    super = clazz;
    do {
        if (!(options.show_flag & SHOW_FIELD))
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

    if (clazz.NumMethods() && (options.show_flag & SHOW_METHOD)) {
        if (needEnd) ENTER();
        LOGI(ANSI_COLOR_LIGHTCYAN "  // Methods:\n" ANSI_COLOR_RESET);
        needEnd = true;
    }
    auto print_method = [](art::ArtMethod& method) -> bool {
        LOGI("    [0x%" PRIx64 "] " ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_RESET "%s\n",
                method.Ptr(), art::PrettyJavaAccessFlags(method.access_flags()).c_str(), method.ColorPrettyMethod().c_str());
        return false;
    };
    if ((options.show_flag & SHOW_METHOD)) Android::ForeachArtMethods(current, print_method);

    LOGI("}\n");
}

void ClassCommand::usage() {
    LOGI("Usage: class [CLASSNAME] [OPTION] [TYPE]\n");
    LOGI("Option:\n");
    LOGI("    -m, --method       show class method\n");
    LOGI("    -i, --impl         show class implements class\n");
    LOGI("    -s, --static       show static field\n");
    LOGI("    -f, --field        show instance field\n");
    LOGI("    -x, --hex          basic type hex print\n");
    LOGI("Type: {--app, --zygote, --image, --fake}\n");
    ENTER();
    LOGI("core-parser> class android.net.wifi.WifiNetworkSpecifier\n");
    LOGI("[0x71c530a0]\n");
    LOGI("public final class android.net.wifi.WifiNetworkSpecifier extends android.net.NetworkSpecifier {\n");
    LOGI("  // Implements:\n");
    LOGI("    android.os.Parcelable\n");
    ENTER();
    LOGI("  // Class static fields:\n");
    LOGI("    [0x104] private final static java.lang.String TAG = 0x0\n");
    LOGI("    [0x100] public final static android.os.Parcelable$Creator CREATOR = 0x0\n");
    ENTER();
    LOGI("  // Object instance fields:\n");
    LOGI("    [0x010] public final android.net.wifi.WifiConfiguration wifiConfiguration\n");
    LOGI("    [0x00c] public final android.os.PatternMatcher ssidPatternMatcher\n");
    LOGI("    [0x008] public final android.util.Pair bssidPatternMatcher\n");
    ENTER();
    LOGI("  // extends android.net.NetworkSpecifier\n");
    ENTER();
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x004] private transient int shadow$_monitor_\n");
    LOGI("    [0x000] private transient java.lang.Class shadow$_klass_\n");
    ENTER();
    LOGI("  // Methods:\n");
    LOGI("    [0x791af097e8c8] static void android.net.wifi.WifiNetworkSpecifier.<clinit>()\n");
    LOGI("    [0x791af097e8f0] public void android.net.wifi.WifiNetworkSpecifier.<init>()\n");
    LOGI("    [0x791af097e918] public void android.net.wifi.WifiNetworkSpecifier.<init>(android.os.PatternMatcher, android.util.Pair, android.net.wifi.WifiConfiguration)\n");
    LOGI("    [0x791af097e940] public boolean android.net.wifi.WifiNetworkSpecifier.canBeSatisfiedBy(android.net.NetworkSpecifier)\n");
    LOGI("    [0x791af097e968] public int android.net.wifi.WifiNetworkSpecifier.describeContents()\n");
    LOGI("    [0x791af097e990] public boolean android.net.wifi.WifiNetworkSpecifier.equals(java.lang.Object)\n");
    LOGI("    [0x791af097e9b8] public int android.net.wifi.WifiNetworkSpecifier.hashCode()\n");
    LOGI("    [0x791af097e9e0] public java.lang.String android.net.wifi.WifiNetworkSpecifier.toString()\n");
    LOGI("    [0x791af097ea08] public void android.net.wifi.WifiNetworkSpecifier.writeToParcel(android.os.Parcel, int)\n");
    ENTER();
    LOGI("core-parser> class android.net.wifi.WifiNetworkSpecifier -f -s\n");
    LOGI("[0x71c530a0]\n");
    LOGI("public final class android.net.wifi.WifiNetworkSpecifier extends android.net.NetworkSpecifier {\n");
    LOGI("  // Class static fields:\n");
    LOGI("    [0x104] private final static java.lang.String TAG = 0x0\n");
    LOGI("    [0x100] public final static android.os.Parcelable$Creator CREATOR = 0x0\n");
    ENTER();
    LOGI("  // Object instance fields:\n");
    LOGI("    [0x010] public final android.net.wifi.WifiConfiguration wifiConfiguration\n");
    LOGI("    [0x00c] public final android.os.PatternMatcher ssidPatternMatcher\n");
    LOGI("    [0x008] public final android.util.Pair bssidPatternMatcher\n");
    ENTER();
    LOGI("  // extends android.net.NetworkSpecifier\n");
    ENTER();
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x004] private transient int shadow$_monitor_\n");
    LOGI("    [0x000] private transient java.lang.Class shadow$_klass_\n");
    ENTER();
    LOGI("core-parser> class android.net.wifi.WifiNetworkSpecifier -m | grep desc\n");
    LOGI("    [0x791af097e968] public int android.net.wifi.WifiNetworkSpecifier.describeContents()\n");
}

