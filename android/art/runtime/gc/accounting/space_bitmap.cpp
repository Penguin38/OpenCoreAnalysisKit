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

#include "api/core.h"
#include "android.h"
#include "runtime/gc/accounting/space_bitmap.h"
#include "runtime/runtime_globals.h"

struct ContinuousSpaceBitmap_OffsetTable __ContinuousSpaceBitmap_offset__;

namespace art {
namespace gc {
namespace accounting {

void ContinuousSpaceBitmap::Init() {
    Android::RegisterSdkListener(Android::O, art::gc::accounting::ContinuousSpaceBitmap::Init26);
    Android::RegisterSdkListener(Android::Q, art::gc::accounting::ContinuousSpaceBitmap::Init29);
}

void ContinuousSpaceBitmap::Init26() {
    if (CoreApi::Bits() == 64) {
        __ContinuousSpaceBitmap_offset__ = {
            .mem_map_ = 0,
            .bitmap_begin_ = 8,
            .bitmap_size_ = 16,
            .heap_begin_ = 24,
        };
    } else {
        __ContinuousSpaceBitmap_offset__ = {
            .mem_map_ = 0,
            .bitmap_begin_ = 4,
            .bitmap_size_ = 8,
            .heap_begin_ = 12,
        };
    }
}

void ContinuousSpaceBitmap::Init29() {
    if (CoreApi::Bits() == 64) {
        __ContinuousSpaceBitmap_offset__ = {
            .mem_map_ = 0,
            .bitmap_begin_ = 72,
            .bitmap_size_ = 80,
            .heap_begin_ = 88,
        };
    } else {
        __ContinuousSpaceBitmap_offset__ = {
            .mem_map_ = 0,
            .bitmap_begin_ = 40,
            .bitmap_size_ = 44,
            .heap_begin_ = 48,
        };
    }
}

void ContinuousSpaceBitmap::VisitMarkedRange(uint64_t visit_begin, uint64_t visit_end,
                                             std::function<bool (mirror::Object& object)> visitor, bool check) {
    api::MemoryRef bitmap_begin_ref(bitmap_begin());
    api::MemoryRef heap_begin_ref(heap_begin());
    heap_begin_ref.Prepare(false);
    int point_bit = CoreApi::GetPointSize();

    uint64_t offset_start = visit_begin - heap_begin_ref.Ptr();
    uint64_t offset_end = visit_end - heap_begin_ref.Ptr();

    uint64_t index_start = OffsetToIndex(offset_start, point_bit);
    uint64_t index_end = OffsetToIndex(offset_end, point_bit);

    uint64_t bit_start = (offset_start / kObjectAlignment) % (kBitsPerByte * point_bit);
    uint64_t bit_end = (offset_end / kObjectAlignment) % (kBitsPerByte * point_bit);

    // Index(begin)  ...    Index(end)
    // [xxxxx???][........][????yyyy]
    //      ^                   ^
    //      |                   #---- Bit of visit_end
    //      #---- Bit of visit_begin
    //

    // Left edge.
    uint64_t left_edge = bitmap_begin_ref.valueOf((index_start * point_bit));
    // Mark of lower bits that are not in range.
    left_edge &= ~((static_cast<uint64_t>(1) << bit_start) - 1);

    // Right edge. Either unique, or left_edge.
    uint64_t right_edge;

    if (index_start < index_end) {
        // Left edge != right edge.

        // Traverse left edge.
        if (left_edge != 0) {
            uint64_t ptr_base = IndexToOffset(index_start, point_bit) + heap_begin_ref.Ptr();
            do {
                uint64_t shift = __builtin_ctzll(left_edge);
                mirror::Object obj(ptr_base + shift * kObjectAlignment, heap_begin_ref);
                if (obj.IsNonLargeValid()) {
                    visitor(obj);
                } else if (check) {
                    LOGE("0x%" PRIx64 " is bad object on [0x%" PRIx64 ", 0x%" PRIx64 ").\n", obj.Ptr(), visit_begin, visit_end);
                }
                left_edge ^= ((static_cast<uint64_t>(1)) << shift);
            } while (left_edge != 0);
        }

        // Traverse the middle, full part.
        for (uint64_t i = index_start + 1; i < index_end; ++i) {
            uint64_t w = bitmap_begin_ref.valueOf((i * point_bit));
            if (w != 0) {
                uint64_t ptr_base = IndexToOffset(i, point_bit) + heap_begin_ref.Ptr();
                // Iterate on the bits set in word `w`, from the least to the most significant bit.
                do {
                    uint64_t shift = __builtin_ctzll(w);
                    mirror::Object obj(ptr_base + shift * kObjectAlignment, heap_begin_ref);
                    if (obj.IsNonLargeValid()) {
                        visitor(obj);
                    } else if (check) {
                        LOGE("0x%" PRIx64 " is bad object on [0x%" PRIx64 ", 0x%" PRIx64 ").\n", obj.Ptr(), visit_begin, visit_end);
                    }
                    w ^= (static_cast<uint64_t>(1)) << shift;
                } while (w != 0);
            }
        }

        // Right edge is unique.
        // But maybe we don't have anything to do: visit_end starts in a new word...
        if (bit_end == 0) {
            // Do not read memory, as it could be after the end of the bitmap.
            right_edge = 0;
        } else {
            right_edge = bitmap_begin_ref.valueOf((index_end * point_bit));
        }
    } else {
        // Right edge = left edge.
        right_edge = left_edge;
    }

    // Right edge handling.
    right_edge &= ((static_cast<uint64_t>(1) << bit_end) - 1);
    if (right_edge != 0) {
        uint64_t ptr_base = IndexToOffset(index_end, point_bit) + heap_begin_ref.Ptr();
        // Iterate on the bits set in word `right_edge`, from the least to the most significant bit.
        do {
            uint64_t shift = __builtin_ctzll(right_edge);
            mirror::Object obj(ptr_base + shift * kObjectAlignment, heap_begin_ref);
            if (obj.IsNonLargeValid()) {
                visitor(obj);
            } else if (check) {
                LOGE("0x%" PRIx64 " is bad object on [0x%" PRIx64 ", 0x%" PRIx64 ").\n", obj.Ptr(), visit_begin, visit_end);
            }
            right_edge ^= (static_cast<uint64_t>(1)) << shift;
        } while (right_edge != 0);
    }
}

uint64_t ContinuousSpaceBitmap::OffsetToIndex(uint64_t offset, int point_bit) {
    return offset / kObjectAlignment / (kBitsPerByte * point_bit);
}

uint64_t ContinuousSpaceBitmap::IndexToOffset(uint64_t index, int point_bit) {
    return index * kObjectAlignment * (kBitsPerByte * point_bit);
}

} // namespace space
} // namespace gc
} // namespace art
