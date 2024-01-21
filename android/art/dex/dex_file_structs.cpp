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

#include "dex/dex_file_structs.h"

struct TypeId_OffsetTable __TypeId_offset__;
struct TypeId_SizeTable __TypeId_size__;
struct StringId_OffsetTable __StringId_offset__;
struct StringId_SizeTable __StringId_size__;
struct FieldId_OffsetTable __FieldId_offset__;
struct FieldId_SizeTable __FieldId_size__;
struct MethodId_OffsetTable __MethodId_offset__;
struct MethodId_SizeTable __MethodId_size__;
struct ProtoId_OffsetTable __ProtoId_offset__;
struct ProtoId_SizeTable __ProtoId_size__;
struct TypeList_OffsetTable __TypeList_offset__;
struct TypeList_SizeTable __TypeList_size__;
struct TypeItem_OffsetTable __TypeItem_offset__;
struct TypeItem_SizeTable __TypeItem_size__;

namespace art {
namespace dex {

void TypeId::Init() {
    __TypeId_offset__ = {
        .descriptor_idx_ = 0,
    };

    __TypeId_size__ = {
        .THIS = 4,
    };
}

void StringId::Init() {
    __StringId_offset__ = {
        .string_data_off_ = 0,
    };

    __StringId_size__ = {
        .THIS = 4,
    };
}

void FieldId::Init() {
     __FieldId_offset__ = {
        .class_idx_ = 0,
        .type_idx_ = 2,
        .name_idx_ = 4,
    };

    __FieldId_size__ = {
        .THIS = 8,
    };
}

void MethodId::Init() {
    __MethodId_offset__ = {
        .class_idx_ = 0,
        .proto_idx_ = 2,
        .name_idx_ = 4,
    };

    __MethodId_size__ = {
        .THIS = 8,
    };
}

void ProtoId::Init() {
    __ProtoId_offset__ = {
        .shorty_idx_ = 0,
        .return_type_idx_ = 4,
        .pad_ = 6,
        .parameters_off_ = 8,
    };

    __ProtoId_size__ = {
        .THIS = 12,
    };
}

void TypeList::Init() {
    __TypeList_offset__ = {
        .size_ = 0,
        .list_ = 4,
    };

    __TypeList_size__ = {
        .THIS = 8,
    };
}

void TypeItem::Init() {
    __TypeItem_offset__ = {
        .type_idx_ = 0,
    };

    __TypeItem_size__ = {
        .THIS = 2,
    };
}

} // namespace dex
} // namespace art
