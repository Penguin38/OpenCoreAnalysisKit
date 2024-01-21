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

#include "dex/utf.h"

namespace art {

void ConvertUtf16ToModifiedUtf8(char* utf8_out, uint64_t byte_count, uint16_t* utf16_in, uint64_t char_count) {
    if (byte_count == char_count) {
        // Common case where all characters are ASCII.
        uint16_t *utf16_end = utf16_in + char_count;
        for (uint16_t *p = utf16_in; p < utf16_end;) {
            *utf8_out++ = static_cast<char>(*p++);
        }
        return;
    }

    // String contains non-ASCII characters.
    while (char_count--) {
        uint16_t ch = *utf16_in++;
        if (ch > 0 && ch <= 0x7f) {
            *utf8_out++ = ch;
        } else {
            // Char_count == 0 here implies we've encountered an unpaired
            // surrogate and we have no choice but to encode it as 3-byte UTF
            // sequence. Note that unpaired surrogates can occur as a part of
            // "normal" operation.
            if ((ch >= 0xd800 && ch <= 0xdbff) && (char_count > 0)) {
                uint16_t ch2 = *utf16_in;

                // Check if the other half of the pair is within the expected
                // range. If it isn't, we will have to emit both "halves" as
                // separate 3 byte sequences.
                if (ch2 >= 0xdc00 && ch2 <= 0xdfff) {
                    utf16_in++;
                    char_count--;
                    uint32_t code_point = (ch << 10) + ch2 - 0x035fdc00;
                    *utf8_out++ = (code_point >> 18) | 0xf0;
                    *utf8_out++ = ((code_point >> 12) & 0x3f) | 0x80;
                    *utf8_out++ = ((code_point >> 6) & 0x3f) | 0x80;
                    *utf8_out++ = (code_point & 0x3f) | 0x80;
                    continue;
                }
            }

            if (ch > 0x07ff) {
                // Three byte encoding.
                *utf8_out++ = (ch >> 12) | 0xe0;
                *utf8_out++ = ((ch >> 6) & 0x3f) | 0x80;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            } else /*(ch > 0x7f || ch == 0)*/ {
                // Two byte encoding.
                *utf8_out++ = (ch >> 6) | 0xc0;
                *utf8_out++ = (ch & 0x3f) | 0x80;
            }
        }
    }
}

uint64_t CountUtf8Bytes(const uint16_t* chars, uint64_t char_count) {
    uint64_t result = 0;
    const uint16_t *end = chars + char_count;
    while (chars < end) {
        const uint16_t ch = *chars++;
        if (ch != 0 && ch < 0x80) {
            result++;
            continue;
        }
        if (ch < 0x800) {
            result += 2;
            continue;
        }
        if (ch >= 0xd800 && ch < 0xdc00) {
            if (chars < end) {
                const uint16_t ch2 = *chars;
                if (ch2 >= 0xdc00 && ch2 < 0xe000) {
                    chars++;
                    result += 4;
                    continue;
                }
            }
        }
        result += 3;
    }
    return result;
}

} // namespace art
