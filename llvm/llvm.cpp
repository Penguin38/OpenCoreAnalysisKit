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

#include "llvm.h"
#include "cxx/string.h"
#include "cxx/vector.h"
#include "cxx/map.h"
#include "cxx/set.h"
#include "cxx/list.h"
#include "cxx/unordered_map.h"
#include "cxx/unordered_set.h"
#include "cxx/deque.h"
#include "cxx/split_buffer.h"
#include "cxx/mutex.h"

void LLVM::Init() {
    cxx::string::Init();
    cxx::vector::Init();
    cxx::map::Init();
    cxx::map::pair::Init();
    cxx::set::Init();
    cxx::set::pair::Init();
    cxx::list::Init();
    cxx::unordered_map::Init();
    cxx::unordered_set::Init();
    cxx::deque::Init();
    cxx::split_buffer::Init();
    cxx::mutex::Init();
}
