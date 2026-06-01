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

#include "base/utils.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <sstream>

std::string Utils::ConvertAscii(uint64_t value, int len) {
    std::string sb;
    uint8_t byte;
    for (int i = 0; i < len; i++) {
        byte = (value >> (8 * i)) & 0xFF;
        if (byte > 0x20 && byte < 0x7F) {
            sb += byte;
        } else {
            sb.append(".");
        }
    }
    return sb;
}

uint64_t Utils::atol(const char* src) {
    uint64_t value;
    sscanf(src, "%" PRIx64 "", &value);
    return value;
}

std::string Utils::ToHex(uint64_t value) {
    std::string sb;
    std::ostringstream ss;
    ss << "0x" << std::hex << value;
    sb.append(ss.str());
    return sb;
}

uint32_t Utils::CRC32(uint8_t* data, uint32_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t k = 0; k < len; ++k) {
        crc ^= (uint32_t)data[k] << 24;
        for (int i = 0; i < 8; ++i) {
            if (crc & (1 << 31)) {
                crc = (crc << 1) ^ 0x04C11DB7;
            } else {
                crc <<= 1;
            }
            crc &= 0xFFFFFFFF;
        }
    }
    return crc;
}

uint64_t Utils::CRC64(uint8_t* data, uint64_t len) {
    uint64_t crc = 0xFFFFFFFFFFFFFFFF;
    for (uint64_t k = 0; k < len; ++k) {
        crc ^= (uint64_t)data[k] << 56;
        for (int i = 0; i < 8; ++i) {
            if (crc & (1ULL << 63)) {
                crc = (crc << 1) ^ 0x42F0E1EBA9EA3693;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}
