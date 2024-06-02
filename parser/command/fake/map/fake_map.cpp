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
#include "api/core.h"
#include "command/fake/map/fake_map.h"

int FakeLinkMap::OptionMap(int argc, char* const argv[]) {
    auto callback = [&](LinkMap* map) -> bool {
        if (map->l_name()) {
            CoreApi::Write(map->l_name(), (void *)map->name(), strlen(map->name()) + 1);
        }
        return false;
    };
    CoreApi::ForeachLinkMap(callback);
    return 0;
}

void FakeLinkMap::Usage() {
    LOGI("Usage: fake map\n");
}
