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

#ifndef CORE_API_BRIDGE_H_
#define CORE_API_BRIDGE_H_

#include "common/load_block.h"

namespace api {
class Bridge {
public:
    static LoadBlock* FindLoadBlock(uint64_t vaddr, bool check);
};
} // namespace api

#endif // CORE_API_BRIDGE_H_
