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
#include "command/cmd_space.h"
#include "api/core.h"
#include "runtime/runtime.h"
#include "runtime/gc/heap.h"
#include "runtime/gc/space/space.h"
#include "runtime/gc/space/large_object_space.h"
#include <unistd.h>
#include <getopt.h>

int SpaceCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    check = false;
    flag = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"check",    no_argument,       0,  'c'},
        {"app",      no_argument,       0,   0 },
        {"zygote",   no_argument,       0,   1 },
        {"image",    no_argument,       0,   2 },
        {"fake",     no_argument,       0,   3 },
    };

    while ((opt = getopt_long(argc, argv, "c",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                check = true;
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

    if (check) {
        auto callback = [&](art::mirror::Object& object) -> bool {
            // do nothing
            return false;
        };
        Android::ForeachObjects(callback, flag, true);
    } else {
        art::Runtime& runtime = art::Runtime::Current();
        art::gc::Heap& heap = runtime.GetHeap();

        LOGI(ANSI_COLOR_LIGHTRED "TYPE   REGION                  ADDRESS             NAME\n" ANSI_COLOR_RESET);
        for (const auto& space : heap.GetContinuousSpaces()) {
            art::gc::space::ContinuousSpace* sp = space.get();
            LOGI(" %2d  " ANSI_COLOR_LIGHTCYAN "[0x%" PRIx64 ", 0x%" PRIx64 ")  " ANSI_COLOR_LIGHTYELLOW "0x%" PRIx64 "  " ANSI_COLOR_LIGHTGREEN "%s\n" ANSI_COLOR_RESET,
                    sp->GetType(), sp->Begin(), sp->End(), sp->Ptr(), sp->GetName());
        }

        for (const auto& space : heap.GetDiscontinuousSpaces()) {
            art::gc::space::DiscontinuousSpace* dsp = space.get();
            if (space->IsLargeObjectSpace()) {
                art::gc::space::LargeObjectSpace *sp = reinterpret_cast<art::gc::space::LargeObjectSpace *>(dsp);
                LOGI(" %2d  " ANSI_COLOR_LIGHTCYAN "[0x%" PRIx64 ", 0x%" PRIx64 ")  " ANSI_COLOR_LIGHTYELLOW "0x%" PRIx64 "  " ANSI_COLOR_LIGHTGREEN "%s\n" ANSI_COLOR_RESET,
                        sp->GetType(), sp->Begin(), sp->End(), sp->Ptr(), sp->GetName());
            } else {
                LOGI(" %2d  [0x0, 0x0)  0x%" PRIx64 "  %s\n", space->GetType(), space->Ptr(), space->GetName());
            }
        }
    }
    return 0;
}

void SpaceCommand::usage() {
    LOGI("Usage: space [OPTION] [TYPE]\n");
    LOGI("Option:\n");
    LOGI("    -c, --check   check space bad object.\n");
    LOGI("Type: {--app, --zygote, --image, --fake}\n");
    ENTER();
    LOGI("core-parser> space\n");
    LOGI("TYPE   REGION                  ADDRESS             NAME\n");
    LOGI("  5  [0x12c00000, 0x2ac00000)  0x75db0d608820  main space (region space)\n");
    LOGI("  0  [0x70209000, 0x7033d840)  0x75dbad608430  /system/framework/x86_64/boot.art\n");
    LOGI("  0  [0x704c2000, 0x704dfc88)  0x75dbad6086f0  /system/framework/x86_64/boot-core-libart.art\n");
    LOGI("  ...\n");
    ENTER();
    LOGI("core-parser> space --check --app\n");
    LOGI("ERROR: Region:[0x12c00000, 0x12c00018) main space (region space) has bad object!!\n");
}
