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

#ifndef ANDROID_ART_RUNTIME_HANDLE_SCOPE_H_
#define ANDROID_ART_RUNTIME_HANDLE_SCOPE_H_

#include "api/memory_ref.h"

struct HandleScope_SizeTable {
    uint32_t THIS;
};

extern struct HandleScope_SizeTable __HandleScope_size__;

namespace art {

class HandleScope : public api::MemoryRef {
public:
    HandleScope(uint64_t v) : api::MemoryRef(v) {}
    HandleScope(const api::MemoryRef& ref) : api::MemoryRef(ref) {}
    HandleScope(uint64_t v, api::MemoryRef& ref) : api::MemoryRef(v, ref) {}
    HandleScope(uint64_t v, api::MemoryRef* ref) : api::MemoryRef(v, ref) {}

    static void Init();
    static uint32_t SizeOf(uint32_t num_references);
};

} //namespace art

#endif // ANDROID_ART_RUNTIME_HANDLE_SCOPE_H_
