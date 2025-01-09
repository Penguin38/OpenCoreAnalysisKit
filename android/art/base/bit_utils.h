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

#ifndef ANDROID_ART_BASE_BIT_UTILS_H__
#define ANDROID_ART_BASE_BIT_UTILS_H__

#include <stdint.h>
#include <limits>
#include <type_traits>

namespace art {

// Like sizeof, but count how many bits a type takes. Pass type explicitly.
template <typename T>
constexpr size_t BitSizeOf() {
    using unsigned_type = std::make_unsigned_t<T>;
    return std::numeric_limits<unsigned_type>::digits;
}

// Like sizeof, but count how many bits a type takes. Infers type from parameter.
template <typename T>
constexpr size_t BitSizeOf(T /*x*/) {
    return BitSizeOf<T>();
}

// Return the number of 1-bits in `x`.
template<typename T>
constexpr int POPCOUNT(T x) {
    return (sizeof(T) == sizeof(uint32_t)) ? __builtin_popcount(x) : __builtin_popcountll(x);
}

// Create a mask for the least significant "bits"
// The returned value is always unsigned to prevent undefined behavior for bitwise ops.
//
// Given 'bits',
// Returns:
//                   <--- bits --->
// +-----------------+------------+
// | 0 ............0 |   1.....1  |
// +-----------------+------------+
// msb                           lsb
template <typename T = size_t>
inline static constexpr std::make_unsigned_t<T> MaskLeastSignificant(size_t bits) {
    using unsigned_T = std::make_unsigned_t<T>;
    if (bits >= BitSizeOf<T>()) {
        return std::numeric_limits<unsigned_T>::max();
    } else {
        auto kOne = static_cast<unsigned_T>(1);  // Do not truncate for T>size_t.
        return static_cast<unsigned_T>((kOne << bits) - kOne);
    }
}

// Extracts the bitfield starting at the least significant bit "lsb" with a bitwidth of 'width'.
// Signed types are sign-extended during extraction. (Equivalent of ARM UBFX/SBFX instruction).
//
// Given:
//           <-- width   -->
// +--------+-------------+-------+
// |        |   bitfield  |       +
// +--------+-------------+-------+
//                       lsb      0
// (Unsigned) Returns:
//                  <-- width   -->
// +----------------+-------------+
// | 0...        0  |   bitfield  |
// +----------------+-------------+
//                                0
// (Signed) Returns:
//                  <-- width   -->
// +----------------+-------------+
// | S...        S  |   bitfield  |
// +----------------+-------------+
//                                0
// where S is the highest bit in 'bitfield'.
template <typename T>
inline static constexpr T BitFieldExtract(T value, size_t lsb, size_t width) {
    const auto val = static_cast<std::make_unsigned_t<T>>(value);

    const T bitfield_unsigned =
        static_cast<T>((val >> lsb) & MaskLeastSignificant<T>(width));
    if (std::is_signed_v<T>) {
        // Perform sign extension
        if (width == 0) {  // Avoid underflow.
            return static_cast<T>(0);
        } else if (bitfield_unsigned & (1 << (width - 1))) {  // Detect if sign bit was set.
            // MSB        <width> LSB
            // 0b11111...100...000000
            const auto ones_negmask = ~MaskLeastSignificant<T>(width);
            return static_cast<T>(bitfield_unsigned | ones_negmask);
        }
    }
    // Skip sign extension.
    return bitfield_unsigned;
}

} // namespace art

#endif // ANDROID_ART_BASE_BIT_UTILS_H__
