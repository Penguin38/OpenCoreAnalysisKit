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

#include "logger/log.h"
#include "common/xz/lzma.h"
#include <stdio.h>
#if defined(__LZMA__)
#include "api/lzma.h"
#endif // __LZMA__

namespace xz {

uint64_t LZMA::TotalSize() {
#if defined(__LZMA__)
    uint64_t total_output_size = 0;
    size_t out_size = 0;
    uint8_t out_buffer[4096];

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK) {
        LOGE("LZMA: Error initializing decoder: %d\n", ret);
        return total_output_size;
    }

    strm.next_in = data();
    strm.avail_in = size();

    // cloc decode size
    do {
        strm.next_out = out_buffer;
        strm.avail_out = sizeof(out_buffer);

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);

        if (ret != LZMA_OK && ret != LZMA_STREAM_END)
            break;

        out_size = sizeof(out_buffer) - strm.avail_out;
        total_output_size += out_size;

        if (ret == LZMA_STREAM_END)
            break;

    } while (strm.avail_out == 0);

    lzma_end(&strm);
    return (ret == LZMA_STREAM_END) ? total_output_size : 0;
#else
    return 0;
#endif // __LZMA__
}

MemoryMap* LZMA::Decode2Map() {
#if defined(__LZMA__)
    MemoryMap* map = nullptr;
    uint64_t current_offset = 0;
    size_t out_size = 0;
    uint8_t out_buffer[4096];

    uint64_t total_size = TotalSize();
    if (!total_size)
        return map;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, 0);
    if (ret != LZMA_OK)
        return map;

    strm.next_in = data();
    strm.avail_in = size();

    map = MemoryMap::MmapZeroMem(total_size);
    do {
        strm.next_out = out_buffer;
        strm.avail_out = sizeof(out_buffer);

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);

        if (ret != LZMA_OK && ret != LZMA_STREAM_END)
            break;

        out_size = sizeof(out_buffer) - strm.avail_out;
        memcpy(reinterpret_cast<void *>(map->data() + current_offset),
               reinterpret_cast<void *>(out_buffer), out_size);
        current_offset += out_size;

        if (ret == LZMA_STREAM_END)
            break;

    } while (strm.avail_out == 0);

end:
    lzma_end(&strm);
    return map;
#else
    return nullptr;
#endif // __LZMA__
}

} // xz
