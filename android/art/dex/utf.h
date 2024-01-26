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

/*
 * Returns the number of modified UTF-8 bytes needed to represent the given
 * UTF-16 string.
 */
size_t CountUtf8Bytes(const uint16_t* chars, size_t char_count);

/*
 * Convert from UTF-16 to Modified UTF-8. Note that the output is _not_
 * NUL-terminated. You probably need to call CountUtf8Bytes before calling
 * this anyway, so if you want a NUL-terminated string, you know where to
 * put the NUL byte.
 */
void ConvertUtf16ToModifiedUtf8(char* utf8_out, size_t byte_count,
                                const uint16_t* utf16_in, size_t char_count);

} // namespace art

#endif  //  ANDROID_ART_DEX_UTF_H_
