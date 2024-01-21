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

#ifndef ANDROID_ART_DEX_UTF_H_
#define ANDROID_ART_DEX_UTF_H_

#include <sys/types.h>
#include <stdint.h>

namespace art {

uint64_t CountUtf8Bytes(const uint16_t* chars, uint64_t char_count);
void ConvertUtf16ToModifiedUtf8(char* utf8_out, uint64_t byte_count, uint16_t* utf16_in, uint64_t char_count);

} // namespace art

#endif  //  ANDROID_ART_DEX_UTF_H_
