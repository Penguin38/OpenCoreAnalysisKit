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
#include "base/utils.h"
#include "common/bit.h"
#include "command/fake/map/fake_map.h"
#include <linux/elf.h>
#include <unistd.h>
#include <getopt.h>
#include <memory>

int FakeLinkMap::AutoCreate32() {
    return 0;
}

int FakeLinkMap::Append32(uint64_t addr, const char* name) {
    return 0;
}

bool FakeLinkMap::FakeLD32(LinkMap* map) {
    return false;
}
