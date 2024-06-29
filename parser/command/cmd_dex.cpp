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
#include "base/utils.h"
#include "command/env.h"
#include "command/cmd_dex.h"
#include "command/command_manager.h"
#include "api/core.h"
#include "android.h"
#include "runtime/runtime.h"
#include "runtime/class_linker.h"
#include <string>
#include <unistd.h>
#include <getopt.h>
#include <filesystem>

int DexCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    dump_dex = false;
    dump_ori = false;
    app = false;
    num = 0;
    dir = const_cast<char *>(Env::CurrentDir());

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"origin",  no_argument,       0,  'o'},
        {"app",     no_argument,       0,   1 },
        {"dir",     required_argument, 0,  'd'},
        {"num",     required_argument, 0,  'n'},
        {0,         0,                 0,   0 }
    };

    while ((opt = getopt_long(argc, argv, "od:n:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'o':
                dump_ori = true;
                break;
            case 1:
                app = true;
                dump_dex = true;
                break;
            case 'd':
                dir = optarg;
                break;
            case 'n':
                num = std::atoi(optarg);
                dump_dex = true;
                break;
        }
    }

    art::Runtime& runtime = art::Runtime::Current();
    art::ClassLinker& linker = runtime.GetClassLinker();
    if (!dump_dex)
        LOGI("NUM DEXCACHE    REGION                   FLAGS NAME\n");
    int pos = 0;
    for (const auto& value : linker.GetDexCacheDatas()) {
        pos++;
        art::mirror::DexCache& dex_cache = value->GetDexCache();
        art::DexFile& dex_file = value->GetDexFile();
        if (!dump_dex) {
            ShowDexCacheRegion(pos, dex_cache, dex_file);
        } else {
            DumpDexFile(pos, dex_cache, dex_file);
        }
    }
    return 0;
}

void DexCommand::ShowDexCacheRegion(int pos, art::mirror::DexCache& dex_cache, art::DexFile& dex_file) {
    std::string name;
    if (dex_cache.Ptr()) {
        name = dex_cache.GetLocation().ToModifiedUtf8();
    } else if (dex_file.Ptr()) {
        name = dex_file.GetLocation().c_str();
    }
    LoadBlock* block = CoreApi::FindLoadBlock(dex_file.data_begin(), false);
    if (block) {
        if (!dump_ori && block->isMmapBlock()) {
            name = block->name();
        } else {
            art::OatDexFile& oat_dex_file = dex_file.GetOatDexFile();
            if (oat_dex_file.Ptr()) {
                art::OatFile& oat_file = oat_dex_file.GetOatFile();
                if (oat_file.Ptr() && block->virtualContains(oat_file.GetVdexBegin())) {
                    name = oat_file.GetVdexFile().GetName();
                }
            }
        }
        std::string valid;
        if (block->isValid()) {
            valid.append("[*]");
            if (block->isOverlayBlock()) {
                valid.append("(OVERLAY)");
            } else if (block->isMmapBlock()) {
                valid.append("(MMAP");
                if (block->GetMmapOffset()) {
                    valid.append(" ");
                    valid.append(Utils::ToHex(block->GetMmapOffset()));
                }
                valid.append(")");
            }
        } else {
            valid.append("[EMPTY]");
        }

        LOGI("%3d 0x%08lx  [%lx, %lx)  %s  %s %s\n",pos, dex_cache.Ptr(), block->vaddr(), block->vaddr() + block->size(),
                block->convertFlags().c_str(), name.c_str(), valid.c_str());
    } else {
        LOGE("ERROR: Unknown DexCache(0x%lx) %s region\n", dex_cache.Ptr(), dex_cache.GetLocation().ToModifiedUtf8().c_str());
    }

}

void DexCommand::DumpDexFile(int pos, art::mirror::DexCache& dex_cache, art::DexFile& dex_file) {
    if (num > 0 && num != pos)
        return;

    std::string name;
    if (dex_cache.Ptr()) {
        name = dex_cache.GetLocation().ToModifiedUtf8();
    } else if (dex_file.Ptr()) {
        name = dex_file.GetLocation().c_str();
    }
    LoadBlock* block = CoreApi::FindLoadBlock(dex_file.data_begin(), false);
    if (block) {
        art::OatDexFile& oat_dex_file = dex_file.GetOatDexFile();
        if (oat_dex_file.Ptr()) {
            art::OatFile& oat_file = oat_dex_file.GetOatFile();
            if (oat_file.Ptr() && block->virtualContains(oat_file.GetVdexBegin())) {
                name = oat_file.GetVdexFile().GetName();
            }
        }

        if (app) {
            if (name[0] == '/' && name.substr(0, 9) != "/data/app")
                return;
        }

        std::filesystem::path file(name);
        std::string fileName = file.filename().string();
        fileName.append("_").append(Utils::ToHex(dex_file.location_checksum()));
        std::string output = dir;
        output.append("/").append(fileName);

        int argc = 6;
        std::string bs = Utils::ToHex(dex_file.data_begin());
        std::string es = Utils::ToHex(dex_file.data_begin() + dex_file.data_size());
        char* argv[6] = {
                const_cast<char*>("rd"),
                const_cast<char*>(bs.c_str()),
                const_cast<char*>("-e"),
                const_cast<char*>(es.c_str()),
                const_cast<char*>("-f"),
                const_cast<char*>(output.c_str())};
        CommandManager::Execute(argv[0], argc, argv);
    }
}

void DexCommand::usage() {
    LOGI("Usage: dex [--origin|-o]\n");
}
