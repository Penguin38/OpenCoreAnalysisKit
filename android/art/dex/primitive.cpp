/*
 * Copyright (C) 2011 The Android Open Source Project
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

#include "dex/primitive.h"

namespace art {

static const char* kTypeNames[] = {
    "PrimNot",
    "PrimBoolean",
    "PrimByte",
    "PrimChar",
    "PrimShort",
    "PrimInt",
    "PrimLong",
    "PrimFloat",
    "PrimDouble",
    "PrimVoid",
};

static const char* kBoxedDescriptors[] = {
    "Ljava/lang/Object;",
    "Ljava/lang/Boolean;",
    "Ljava/lang/Byte;",
    "Ljava/lang/Character;",
    "Ljava/lang/Short;",
    "Ljava/lang/Integer;",
    "Ljava/lang/Long;",
    "Ljava/lang/Float;",
    "Ljava/lang/Double;",
    "Ljava/lang/Void;",
};

const char* Primitive::PrettyDescriptor(Primitive::Type type) {
    return kTypeNames[type];
}

const char* Primitive::BoxedDescriptor(Primitive::Type type) {
    return kBoxedDescriptors[type];
}

}  // namespace art
