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

    if (optind >= argc)
        return 0;

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
    LOGI("Usage: sysroot <DIR_PATH>[:<PATH>:<PATH>] [OPTION]\n");
    LOGI("Option:\n");
    LOGI("    --map   set sysroot link_map\n");
    LOGI("    --dex   set sysroot dex_cache\n");
    ENTER();
    LOGI("core-parser> sysroot /system:/apex --map\n");
    LOGI("Mmap segment [60969cb26000, 60969cb28000) /system/bin/app_process64 [0]\n");
    LOGI("Mmap segment [60969cb28000, 60969cb2a000) /system/bin/app_process64 [1000]\n");
    LOGI("Mmap segment [75dc65a44000, 75dc65a8b000) /system/bin/linker64 [0]\n");
    LOGI("Mmap segment [75dc65a8b000, 75dc65b8e000) /system/bin/linker64 [46000]\n");
    LOGI("Mmap segment [75dc4d4f8000, 75dc4d5d5000) /system/lib64/libandroid_runtime.so [0]\n");
    LOGI("...\n");
    ENTER();
    LOGI("core-parser> sysroot /system:/apex --dex\n");
    LOGI("Mmap segment [75dc65357000, 75dc6539f000) /system/framework/ext.jar [0]\n");
    LOGI("Mmap segment [75d9a7c00000, 75d9a7fb8000) /system/framework/telephony-common.jar [0]\n");
    LOGI("Mmap segment [75dc658c1000, 75dc658c2000) /system/framework/framework-graphics.jar [0]\n");
    LOGI("Mmap segment [75d9a8f9a000, 75d9a90e8000) /system/framework/framework.jar [24d8000]\n");
    LOGI("Mmap segment [75d9a856d000, 75d9a8600000) /system/framework/ims-common.jar [0]\n");
    LOGI("Mmap segment [75d9a3fa8000, 75d9a4975000) /system/framework/framework.jar [11a8000]\n");
    LOGI("...\n");
}
