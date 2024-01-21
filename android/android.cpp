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

#include "android.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/string.h"
#include "runtime/mirror/array.h"
#include "runtime/mirror/dex_cache.h"
#include "dex/dex_file.h"
#include "dex/dex_file_structs.h"

void Android::Init() {
    art::mirror::Object::Init();
    art::mirror::Class::Init();
    art::mirror::String::Init();
    art::mirror::Array::Init();
    art::mirror::DexCache::Init();
    art::DexFile::Init();
    art::dex::TypeId::Init();
    art::dex::StringId::Init();
    art::dex::FieldId::Init();
    art::dex::MethodId::Init();
    art::dex::ProtoId::Init();
    art::dex::TypeList::Init();
    art::dex::TypeItem::Init();
}
