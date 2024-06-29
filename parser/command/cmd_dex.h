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

#ifndef PARSER_COMMAND_CMD_DEX_H_
#define PARSER_COMMAND_CMD_DEX_H_

#include "command/command.h"
#include "runtime/mirror/dex_cache.h"

class DexCommand : public Command {
public:
    DexCommand() : Command("dex") {}
    ~DexCommand() {}
    int main(int argc, char* const argv[]);
    void usage();
    void DexCachesDump();
    void DexCachesDump_v33();
    void ShowDexCacheRegion(int pos, art::mirror::DexCache& dex_cache, art::DexFile& dex_file);
    void DumpDexFile(int pos, art::mirror::DexCache& dex_cache, art::DexFile& dex_file);
private:
    bool dump_ori = false;
    int num = 0;
    bool app = false;
    char* dir = nullptr;
    bool dump_dex = false;
};

#endif // PARSER_COMMAND_CMD_DEX_H_
