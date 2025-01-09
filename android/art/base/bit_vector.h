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

#ifndef ANDROID_ART_BASE_BIT_VECTOR_H_
#define ANDROID_ART_BASE_BIT_VECTOR_H_

#include "api/memory_ref.h"
#include "base/bit_utils.h"
#include "common/bit.h"

namespace art {

class BitVector {
public:
    static constexpr uint32_t kWordBytes = sizeof(uint32_t);
    static constexpr uint32_t kWordBits = kWordBytes * 8;

    static constexpr uint32_t WordIndex(uint32_t idx) {
        return idx >> 5;
    }

    static constexpr uint32_t BitMask(uint32_t idx) {
        return 1 << (idx & 0x1f);
    }

    static constexpr bool IsBitSet(api::MemoryRef& storage, uint32_t idx) {
        return (storage.value32Of(WordIndex(idx) * sizeof(uint32_t)) & BitMask(idx)) != 0;
    }

    static constexpr uint32_t NumSetBits(api::MemoryRef& storage, uint32_t end) {
        uint32_t word_end = WordIndex(end);
        uint32_t partial_word_bits = end & 0x1f;

        uint32_t count = 0u;
        for (uint32_t word = 0u; word < word_end; word++) {
            count += POPCOUNT(storage.value32Of(word * sizeof(uint32_t)));
        }
        if (partial_word_bits != 0u) {
            count += POPCOUNT(storage.value32Of(word_end * sizeof(uint32_t)) & ~(0xffffffffu << partial_word_bits));
        }
        return count;
    }

    static constexpr uint32_t BitsToWords(uint32_t bits) {
        return RoundUp(bits, kWordBits) / kWordBits;
    }
};

} // namespace art

#endif // ANDROID_ART_BASE_BIT_VECTOR_H_
