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
#include "common/exception.h"
#include "command/cmd_reference.h"
#include "api/core.h"
#include "android.h"
#include <unistd.h>
#include <getopt.h>

int ReferenceCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    bool dump_refs = true;
    int flags = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"global",       no_argument,       0,  0 },
        {"weak",         no_argument,       0,  1 },
    };

    while ((opt = getopt_long(argc, argv, "01",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                dump_refs = true;
                flags |= Android::EACH_GLOBAL_REFERENCES;
                break;
            case 1:
                dump_refs = true;
                flags |= Android::EACH_WEAK_GLOBAL_REFERENCES;
                break;
        }
    }

    if (!flags) {
        flags |= Android::EACH_GLOBAL_REFERENCES;
        flags |= Android::EACH_WEAK_GLOBAL_REFERENCES;
    }

    auto callback = [&](art::mirror::Object& object, uint64_t idx) -> bool {
        std::string descriptor;
        art::mirror::Class thiz = 0x0;
        if (object.IsClass()) {
            thiz = object;
        } else {
            thiz = object.GetClass();
        }
        descriptor = thiz.PrettyDescriptor();

        LOGI("[%ld] " ANSI_COLOR_LIGHTYELLOW  "0x%lx" ANSI_COLOR_LIGHTCYAN " %s\n" ANSI_COLOR_RESET,
                idx, object.Ptr(), descriptor.c_str());

        return false;
    };

    try {
        Android::ForeachReferences(callback, flags);
    } catch(InvalidAddressException e) {
        LOGW("The statistical process was interrupted!\n");
    }

    return 0;
}

void ReferenceCommand::usage() {
    LOGI("Usage: reference|ref [<ID>] [OPTIONE]\n");
    LOGI("Option:\n");
    LOGI("    --global\n");
    LOGI("    --weak-global\n");
}
