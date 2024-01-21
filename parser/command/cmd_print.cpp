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
#include "base/utils.h"
#include "api/core.h"
#include "runtime/runtime_globals.h"
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <iomanip>

int PrintCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !argc)
        return 0;

    bool binary = false;
    bool reference = false;
    int deep = 0;

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"--binary",  no_argument,       0,  'b'},
        {"--ref",     required_argument, 0,  'r'},
        {0,           0,                 0,   0 }
    };
    
    while ((opt = getopt_long(argc, argv, "br::",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'b':
                binary = true;
                break;
            case 'r':
                reference = true;
                if (optind < argc) deep = atoi(optarg);
                break;
        }
    }

    // reset
    optind = 0;

    // symbols init for later
    Android::Init();

    art::mirror::Object ref = Utils::atol(argv[0]);
    DumpObject(ref, binary, reference, deep);
    return 0;
}

void PrintCommand::DumpObject(art::mirror::Object& object, bool binary, bool ref, int deep) {
    uint64_t size = object.SizeOf();
    uint64_t real_size = RoundUp(size, art::kObjectAlignment);
    LOGI("Size: 0x%lx\n", real_size);
    if (size != real_size) {
        LOGI("Padding: 0x%lx\n", real_size - size);
    }

    art::mirror::Class clazz = object.GetClass();
    if (clazz.Ptr()) {
        if (object.IsClass()) {
            art::mirror::Class thiz = object;
            DumpClass(thiz);
        } else if (clazz.IsArrayClass()) {
            art::mirror::Array thiz = object;
            DumpArray(thiz);
        } else {
            DumpInstance(object);
        }
    }
}

void PrintCommand::DumpClass(art::mirror::Class& clazz) {
    LOGI("%s\n", __func__);
    LOGI("Class Name: %s\n", clazz.PrettyDescriptor().c_str());
}

void PrintCommand::DumpArray(art::mirror::Array& array) {
    LOGI("%s\n", __func__);
    art::mirror::Class clazz = array.GetClass();
    LOGI("Array Name: %s\n", clazz.PrettyDescriptor().c_str());
}

void PrintCommand::DumpInstance(art::mirror::Object& object) {
    LOGI("%s\n", __func__);
    art::mirror::Class clazz = object.GetClass();
    LOGI("Object Name: %s\n", clazz.PrettyDescriptor().c_str());
}

void PrintCommand::usage() {
    LOGI("Usage: print|p object -[br]\n");
}
