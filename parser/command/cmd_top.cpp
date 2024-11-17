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
#include "runtime/mirror/class.h"
#include "command/command_manager.h"
#include "command/cmd_top.h"
#include "sun/misc/Cleaner.h"
#include "libcore/util/NativeAllocationRegistry.h"
#include "api/core.h"
#include "android.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sstream>
#include <regex>
#include <map>
#include <vector>

int TopCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return 0;

    num = atoi(argv[1]);
    order = ORDERBY_ALLOC;
    show = false;
    flag = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"alloc",      no_argument,       0,  'a'},
        {"shallow",    no_argument,       0,  's'},
        {"native",     no_argument,       0,  'n'},
        {"display",    no_argument,       0,  'd'},
        {"app",        no_argument,       0,   0 },
        {"zygote",     no_argument,       0,   1 },
        {"image",      no_argument,       0,   2 },
        {"fake",       no_argument,       0,   3 },
    };

    while ((opt = getopt_long(argc, argv, "asnd",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                order = ORDERBY_ALLOC;
                break;
            case 's':
                order = ORDERBY_SHALLOW;
                break;
            case 'n':
                order = ORDERBY_NATIVE;
                break;
            case 'd':
                show = true;
                break;
            case 0:
                flag |= Android::EACH_APP_OBJECTS;
                break;
            case 1:
                flag |= Android::EACH_ZYGOTE_OBJECTS;
                break;
            case 2:
                flag |= Android::EACH_IMAGE_OBJECTS;
                break;
            case 3:
                flag |= Android::EACH_FAKE_OBJECTS;
                break;
        }
    }

    if (!flag) {
        flag |= Android::EACH_APP_OBJECTS;
        flag |= Android::EACH_ZYGOTE_OBJECTS;
        flag |= Android::EACH_IMAGE_OBJECTS;
        flag |= Android::EACH_FAKE_OBJECTS;
    }
    std::map<art::mirror::Class, TopCommand::Pair> classes;
    art::mirror::Class cleaner = 0;
    std::vector<art::mirror::Object> cleaners;
    auto callback = [&](art::mirror::Object& object) -> bool {
        if (object.IsClass())
            return false;

        art::mirror::Class thiz = object.GetClass();
        if (!cleaner.Ptr()) {
            if (thiz.PrettyDescriptor() == "sun.misc.Cleaner") {
                cleaner = thiz;
                cleaners.push_back(object);
            }
        } else if (cleaner == thiz) {
            cleaners.push_back(object);
        }

        auto it = classes.find(thiz.Ptr());
        if (it == classes.end()) {
            TopCommand::Pair pair = {
                .alloc_count = 1,
                .shallow_size = object.SizeOf(),
            };
            classes.insert(std::pair<art::mirror::Class, TopCommand::Pair>(thiz, pair));
        } else {
            TopCommand::Pair& pair = it->second;
            pair.alloc_count += 1;
            pair.shallow_size += object.SizeOf();
        }

        return false;
    };
    Android::ForeachObjects(callback, flag, false);

    LOGI(ANSI_COLOR_LIGHTRED "Address       Allocations      ShallowSize        NativeSize     %s\n" ANSI_COLOR_RESET, show ? "ClassName" : "");
    art::mirror::Class cur_max_thiz = 0;
    TopCommand::Pair cur_max_pair = {
        .alloc_count = 0,
        .shallow_size = 0,
        .native_size = 0,
    };

    for (int i = 0; i < cleaners.size(); i++) {
        sun::misc::Cleaner cleaner = cleaners[i];
        java::lang::Object referent = cleaner.getReferent();

        if (referent.isNull())
            continue;

        auto it = classes.find(referent.klass());
        if (it == classes.end())
            continue;

        try {
            libcore::util::NativeAllocationRegistry::CleanerThunk thunk = cleaner.getThunk();
            if (thunk.isNull())
                continue;

            libcore::util::NativeAllocationRegistry registry = thunk.getRegistry();
            if (registry.isNull())
                continue;

            TopCommand::Pair& pair = it->second;
            pair.native_size += registry.getSize();
        } catch (InvalidAddressException e) {}
    }

    uint64_t total_count = 0;
    uint64_t total_shallow = 0;
    uint64_t total_native = 0;

    for (const auto& value : classes) {
        const art::mirror::Class& thiz = value.first;
        const TopCommand::Pair& pair = value.second;

        total_count += pair.alloc_count;
        total_shallow += pair.shallow_size;
        total_native += pair.native_size;
    }

    LOGI("TOTAL            " ANSI_COLOR_LIGHTMAGENTA "%8ld      " ANSI_COLOR_LIGHTBLUE "%11ld       " ANSI_COLOR_LIGHTGREEN "%11ld\n" ANSI_COLOR_RESET,
         total_count, total_shallow, total_native);
    LOGI("------------------------------------------------------------\n");

    for (int i = 0; i < num; ++i) {
        for (const auto& value : classes) {
            const art::mirror::Class& thiz = value.first;
            const TopCommand::Pair& pair = value.second;

            switch (order) {
                case ORDERBY_ALLOC: {
                   if (pair.alloc_count >= cur_max_pair.alloc_count) {
                       cur_max_thiz = thiz;
                       cur_max_pair = pair;
                   }
                } break;
                case ORDERBY_SHALLOW: {
                   if (pair.shallow_size >= cur_max_pair.shallow_size) {
                       cur_max_thiz = thiz;
                       cur_max_pair = pair;
                   }
                } break;
                case ORDERBY_NATIVE: {
                   if (pair.native_size >= cur_max_pair.native_size) {
                       cur_max_thiz = thiz;
                       cur_max_pair = pair;
                   }
                } break;
            }
        }

        if (!cur_max_thiz.Ptr())
            break;

        LOGI(ANSI_COLOR_LIGHTYELLOW "0x%08lx" ANSI_COLOR_RESET "       " "%8ld      " "%11ld       " "%11ld     " ANSI_COLOR_LIGHTCYAN "%s\n" ANSI_COLOR_RESET,
             cur_max_thiz.Ptr(), cur_max_pair.alloc_count,
             cur_max_pair.shallow_size, cur_max_pair.native_size,
             show ? cur_max_thiz.PrettyDescriptor().c_str() : "");

        classes.erase(cur_max_thiz);
        cur_max_thiz = 0;
        cur_max_pair = {0, 0, 0};
    }
    return 0;
}

void TopCommand::usage() {
    LOGI("Usage: top <NUM> [OPTION] [TYPE]\n");
    LOGI("Option:\n");
    LOGI("    -a, --alloc     order by allocation\n");
    LOGI("    -s, --shallow   order by shallow\n");
    LOGI("    -n, --native    order by native\n");
    LOGI("    -d, --display   show class name\n");
    LOGI("Type: {--app, --zygote, --image, --fake}\n");
    ENTER();
    LOGI("core-parser> top 10 -d\n");
    LOGI("Address       Allocations      ShallowSize        NativeSize     ClassName\n");
    LOGI("TOTAL              136939          8045084            108415\n");
    LOGI("------------------------------------------------------------\n");
    LOGI("0x6f817d58          43562          2629504                 0     java.lang.String\n");
    LOGI("0x6f7fdd30          14281          1405792                 0     long[]\n");
    LOGI("0x6f7992c0          12084           479956                 0     java.lang.Object[]\n");
    LOGI("0x6f824fd0           9405           225720                 0     java.util.HashMap$Node\n");
    LOGI("0x6f7fda18           4689          1033816                 0     int[]\n");
    LOGI("0x6f7fa7b0           3457           110624                 0     java.lang.ref.SoftReference\n");
    LOGI("0x6f835118           3381            40572                 0     java.lang.Integer\n");
    LOGI("0x6f8420e8           2684            85888                 0     java.util.LinkedHashMap$LinkedHashMapEntry\n");
    LOGI("0x6fab25d0           2293            45860                 0     android.icu.util.CaseInsensitiveString\n");
    LOGI("0x6f865b80           1859            37180                 0     java.util.ArrayList\n");
    ENTER();
    LOGI("core-parser> top 10 -d -s --app\n");
    LOGI("Address       Allocations      ShallowSize        NativeSize     ClassName\n");
    LOGI("TOTAL                7592          1943315            104175\n");
    LOGI("------------------------------------------------------------\n");
    LOGI("0x6f7fda18            322           561344                 0     int[]\n");
    LOGI("0x6f817d58           1504           509408                 0     java.lang.String\n");
    LOGI("0x6f799100              9           299177                 0     byte[]\n");
    LOGI("0x6f7fd688             18           247138                 0     char[]\n");
    LOGI("0x6f7fdd30            390           113576                 0     long[]\n");
    LOGI("0x6f7992c0            577            27196                 0     java.lang.Object[]\n");
    LOGI("0x6f865b80            335             6700                 0     java.util.ArrayList\n");
    LOGI("0x6f79ba88            174             6264                 0     sun.misc.Cleaner\n");
    LOGI("0x70101c18            258             6192                 0     android.graphics.Rect\n");
    LOGI("0x70360328             40             5600                 0     android.animation.ObjectAnimator\n");
}
