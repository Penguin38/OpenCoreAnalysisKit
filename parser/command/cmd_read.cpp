/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "command/cmd_read.h"
#include "base/utils.h"
#include "api/core.h"
#include <unistd.h>
#include <getopt.h>
#include <iomanip>

int ReadCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !argc)
        return 0;

    int opt;
    uint64_t begin = Utils::atol(argv[0]);
    uint64_t end = 0;
    char* filepath = nullptr;
    int read_opt = OPT_READ_ALL;

    int option_index = 0;
    static struct option long_options[] = {
        {"origin",  no_argument,       0,  0 },
        {"mmap",    no_argument,       0,  1 },
        {"overlay", no_argument,       0,  2 },
        {"end",     required_argument, 0, 'e'},
        {"file",    required_argument, 0, 'f'},
        {0,         0,                 0,  0 }
    };
    
    while ((opt = getopt_long(argc, argv, "e:f:012",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'e':
                end = Utils::atol(optarg);
                break;
            case 'f':
                filepath = optarg;
                break;
            case 0:
                read_opt = OPT_READ_OR;
                break;
            case 1:
                read_opt = OPT_READ_MMAP;
                break;
            case 2:
                read_opt = OPT_READ_OVERLAY;
                break;
        }
    }

    // reset
    optind = 0;

    LoadBlock* block = CoreApi::FindLoadBlock(begin);
    if (block && end > (block->vaddr() + block->size()))
        end = block->vaddr() + block->size();

    int count = (end - begin) / 8;
    if (begin >= end || !count) {
        uint64_t* value = reinterpret_cast<uint64_t *>(CoreApi::GetReal(begin, read_opt));
        if (value) {
            if (!filepath) {
                std::string ascii = Utils::ConvertAscii(*value, 8);
                LOGI("%lx: %016lx  %s\n", begin, (*value), ascii.c_str());
            } else {
                saveBinary(filepath, value, end - begin);
            }
        }
    } else {
        uint8_t* buf = reinterpret_cast<uint8_t*>(malloc(count * 8));
        if (CoreApi::Read(begin, count * 8, buf, read_opt)) {
            uint64_t* value = reinterpret_cast<uint64_t *>(buf);
            if (!filepath) {
                for (int i = 0; i < count; i += 2) {
                    LOGI("%lx: %016lx  %016lx  %s%s\n", (begin + i * 8), value[i], value[i + 1],
                            Utils::ConvertAscii(value[i], 8).c_str(), Utils::ConvertAscii(value[i + 1], 8).c_str());
                }
            } else {
                saveBinary(filepath, value, end - begin);
            }
        }
        free(buf);
    }
    return 0;
}

void ReadCommand::saveBinary(char* path, uint64_t* real, uint64_t size) {
    if (path) {
        FILE* fp = fopen(path, "wb");
        if (fp) {
            fwrite(real, size, 1, fp);
            fclose(fp);
            LOGI("Saved [%s].\n", path);
        }
    }
}

void ReadCommand::usage() {
    LOGI("Usage: read|rd begin [-e end] [--opt] [-f path]\n");
    LOGI("         opt: --origin --mmap --overlay\n");
}

