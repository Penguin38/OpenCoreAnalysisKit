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
#include "base/macros.h"
#include "command/cmd_method.h"
#include "runtime/art_method.h"
#include "base/utils.h"
#include "dalvik_vm_bytecode.h"
#include <unistd.h>
#include <getopt.h>
#include <iomanip>

int MethodCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady()
            || !Android::IsSdkReady()
            || !(argc > 1))
        return 0;

    int opt;
    uint64_t artptr = Utils::atol(argv[1]) & CoreApi::GetVabitsMask();
    int dump_opt = METHOD_DUMP_NAME;

    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"--dex-dump",    no_argument,       0,  0 },
        {"--oat-dump",    no_argument,       0,  1 },
        {"--dex-inst",    required_argument, 0, 'i'},
        {"--num",         required_argument, 0, 'n'},
        {0,               0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "i:n:01",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'i':
                break;
            case 'n':
                break;
            case 0:
                dump_opt |= METHOD_DUMP_DEXCODE;
                break;
            case 1:
                dump_opt |= METHOD_DUMP_OATCODE;
                break;
        }
    }

    art::ArtMethod method = artptr;
    uint32_t dex_method_idx = method.GetDexMethodIndex();
    if (LIKELY(dex_method_idx != art::dex::kDexNoIndex)) {
        LOGI("%s%s [dex_method_idx=%d]\n", art::PrettyJavaAccessFlags(method.access_flags()).c_str(),
                       method.PrettyMethod().c_str(), dex_method_idx);

        if (dump_opt & METHOD_DUMP_DEXCODE) {

        }

        if (dump_opt & METHOD_DUMP_OATCODE) {

        }
    } else {
        LOGI("%s\n", method.PrettyMethod().c_str());
    }
    return 0;
}

void MethodCommand::usage() {
    LOGI("Usage: method\n");
}
