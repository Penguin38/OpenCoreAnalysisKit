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

#include "runtime/mirror/dex_cache.h"

struct DexCache_OffsetTable __DexCache_offset__;
struct DexCache_SizeTable __DexCache_size__;

namespace art {
namespace mirror {

void DexCache::Init26() {
    __DexCache_offset__ = {
        .location_ = 8,
        .dex_file_ = 16,
        .preresolved_strings_ = 24,
        .resolved_call_sites_ = 32,
        .resolved_fields_ = 40,
        .resolved_method_types_ = 48,
        .resolved_methods_ = 56,
        .resolved_types_ = 64,
        .strings_ = 72,
        .num_preresolved_strings_ = 12,
        .num_resolved_call_sites_ = 80,
        .num_resolved_fields_ = 84,
        .num_resolved_method_types_ = 88,
        .num_resolved_methods_ = 92,
        .num_resolved_types_ = 96,
        .num_strings_ = 100,
    };

    __DexCache_size__ = {
        .THIS = 104,
    };
}

void DexCache::Init30() {
    __DexCache_offset__ = {
        .class_loader_ = 8,
        .location_ = 12,
        .dex_file_ = 16,
        .preresolved_strings_ = 24,
        .resolved_call_sites_ = 32,
        .resolved_fields_ = 40,
        .resolved_method_types_ = 48,
        .resolved_methods_ = 56,
        .resolved_types_ = 64,
        .strings_ = 72,
        .num_preresolved_strings_ = 80,
        .num_resolved_call_sites_ = 84,
        .num_resolved_fields_ = 88,
        .num_resolved_method_types_ = 92,
        .num_resolved_methods_ = 96,
        .num_resolved_types_ = 100,
        .num_strings_ = 104,
    };

    __DexCache_size__ = {
        .THIS = 108,
    };
}

} // namespace mirror
} // namespace art
