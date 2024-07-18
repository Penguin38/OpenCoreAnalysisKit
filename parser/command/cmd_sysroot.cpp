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
#include "command/cmd_sysroot.h"
#include "api/core.h"
#include "android.h"
#include <unistd.h>
#include <getopt.h>

int SysRootCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !(argc > 1))
        return 0;

    int opt;
    int root = MAP_ROOT | DEX_ROOT;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"map",  no_argument,       0,  0 },
        {"dex",  no_argument,       0,  1 },
        {0,      0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "01",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                root &= ~DEX_ROOT;
                root |= MAP_ROOT;
                break;
            case 1:
                root &= ~MAP_ROOT;
                root |= DEX_ROOT;
                break;
        }
    }

    if (root & MAP_ROOT)
        CoreApi::SysRoot(argv[optind]);

    if (root & DEX_ROOT) {
        if (Android::IsSdkReady()) {
            Android::SysRoot(argv[optind]);
        } else {
            LOGW("Android sdk no ready, You can enter command:\n         env config --sdk <version>\n");
        }
    }

    return 0;
}

void SysRootCommand::usage() {
    LOGI("Usage: sysroot /system:/apex:/vendor --[map|dex]\n");
}
