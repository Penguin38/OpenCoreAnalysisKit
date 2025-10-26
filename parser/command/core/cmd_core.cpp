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
#include "command/env.h"
#include "command/core/cmd_core.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>

int CoreCommand::Load(const char* path) {
    return Load(path, false);
}

int CoreCommand::Load(const char* path, bool remote) {
    return CoreApi::Load(path, remote, Env::Init);
}

int CoreCommand::main(int argc, char* const argv[]) {
    bool need_clean = false;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"clean",  no_argument,       0, 'c'},
        {0,         0,                0,  0 },
    };

    while ((opt = getopt_long(argc, argv, "c",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                need_clean = true;
                break;
        }
    }

    if (need_clean) {
        if (CoreApi::IsReady()) {
            CoreApi::UnLoad();
        }
        return 0;
    }
    return Load(argv[optind]);
}

void CoreCommand::usage() {
    LOGI("Usage: core /tmp/default.core\n");
}
