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
#include "command/cmd_search.h"
#include "api/core.h"
#include "android.h"
#include <unistd.h>
#include <getopt.h>
#include <regex>

int SearchCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !argc)
        return 0;

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"object",   no_argument,       0,  'o'},
        {"class",    no_argument,       0,  'c'},
        {"regex",    no_argument,       0,  'r'},
    };

    int search_flag;
    bool need_regex = false;
    while ((opt = getopt_long(argc, argv, "ocr",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                search_flag |= SEARCH_OBJECT;
                break;
            case 'c':
                search_flag |= SEARCH_CLASS;
                break;
            case 'r':
                need_regex = true;
                break;
        }
    }

    // reset
    optind = 0;
    total_objects = 0;
    const char* classname = argv[0];
    if (!search_flag) search_flag = SEARCH_OBJECT | SEARCH_CLASS;
    auto callback = [&](art::mirror::Object& object) -> bool {
        return SearchObjects(classname, object, search_flag, need_regex);
    };
    Android::ForeachObjects(callback);
    return 0;
}

bool SearchCommand::SearchObjects(const char* classsname, art::mirror::Object& object, int flag, bool regex) {
    int mask = object.IsClass() ? SEARCH_CLASS : SEARCH_OBJECT;
    if (!(flag & mask))
        return false;

    std::string descriptor;
    art::mirror::Class thiz = 0x0;
    if (object.IsClass()) {
        thiz = object;
    } else {
        thiz = object.GetClass();
    }
    descriptor = thiz.PrettyDescriptor();

    if (regex) {
        if (std::regex_search(descriptor, std::regex(classsname))) {
            total_objects++;
            LOGI("[%ld] 0x%lx %s\n", total_objects, object.Ptr(), descriptor.c_str());
        }
    } else {
        if (descriptor == classsname) {
            total_objects++;
            LOGI("[%ld] 0x%lx %s\n", total_objects, object.Ptr(), descriptor.c_str());
        }
    }

    return false;
}

void SearchCommand::usage() {
    LOGI("Usage: search [CLASSNAME] [-r|--regex] [-o|--object] [-c|--class]\n");
}

