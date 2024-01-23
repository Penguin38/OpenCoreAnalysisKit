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

#ifndef ANDROID_ART_RUNTIME_RUNTIME_GLOBALS_H_
#define ANDROID_ART_RUNTIME_RUNTIME_GLOBALS_H_

#include "base/globals.h"

namespace art {

// Size of Dex virtual registers.
static constexpr size_t kVRegSize = 4;

// Required object alignment
static constexpr size_t kObjectAlignmentShift = 3;
static constexpr size_t kObjectAlignment = 1u << kObjectAlignmentShift;
static constexpr size_t kLargeObjectAlignment = kPageSize;

} // namespace art

#endif  // ANDROID_ART_RUNTIME_RUNTIME_GLOBALS_H_
