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

#ifndef ANDROID_ART_BASE_GLOBALS_H_
#define ANDROID_ART_BASE_GLOBALS_H_

#include <sys/types.h>

namespace art {

static const size_t kPageSize = 0x1000;
static const size_t kStackAlignment = 16;
static const size_t kBitsPerByte = 8;
static const size_t kBitsPerByteLog2 = 3;
static const int kBitsPerIntPtrT = 8 * kBitsPerByte;

} // namespace art

#endif  // ANDROID_ART_BASE_GLOBALS_H_
