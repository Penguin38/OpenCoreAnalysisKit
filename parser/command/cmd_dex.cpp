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
        LOGI(ANSI_COLOR_LIGHTRED "NUM DEXCACHE    REGION                   FLAGS NAME\n" ANSI_COLOR_RESET);
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

std::string DexCommand::DexFileLocation(art::DexFile& dex_file, bool dump_ori) {
    std::string name;
    if (dex_file.Ptr()) name = dex_file.GetLocation().c_str();
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
    }
    return name;
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

        LOGI("%3d " ANSI_COLOR_LIGHTYELLOW "0x%08lx" ANSI_COLOR_LIGHTCYAN "  [%lx, %lx)" ANSI_COLOR_RESET "  %s  " ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_RESET " %s\n",
                pos, dex_cache.Ptr(), block->vaddr(), block->vaddr() + block->size(),
                block->convertFlags().c_str(), name.c_str(), block->convertValids().c_str());
    } else {
        LOGE("Unknown DexCache(0x%lx) %s region\n", dex_cache.Ptr(), dex_cache.GetLocation().ToModifiedUtf8().c_str());
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
            if (name[0] == '/'
                    && name.substr(0, 9) != "/data/app"
                    && name.substr(0, 10) != "/data/user"
                    && name.substr(0, 10) != "/data/data")
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
    LOGI("Usage: dex [OPTIONE...]\n");
    LOGI("Option:\n");
    LOGI("    -o, --origin           show dex origin name\n");
    LOGI("        --app              dex unpack from app\n");
    LOGI("    -n, --num <NUM>        dex unpack with num\n");
    LOGI("    -d, --dir <DIR_PATH>   unpack output path\n");
    ENTER();
    LOGI("core-parser> dex\n");
    LOGI("NUM DEXCACHE    REGION                   FLAGS NAME\n");
    LOGI("  1 0x6f79ca38  [79185c1ac000, 79185c66a000)  r--  /apex/com.android.art/javalib/core-oj.jar [*]\n");
    LOGI("  2 0x6fa28ad8  [791af2506000, 791af25a8000)  r--  /apex/com.android.art/javalib/core-libart.jar [*]\n");
    LOGI("  3 0x6fa838b8  [79185bf30000, 79185c1ac000)  r--  /apex/com.android.art/javalib/core-icu4j.jar [*]\n");
    LOGI("  4 0x6fb4f5e0  [791af2425000, 791af2488000)  r--  /apex/com.android.art/javalib/okhttp.jar [*]\n");
    LOGI("  5 0x6fb86100  [791af2166000, 791af22bd000)  r--  /apex/com.android.art/javalib/bouncycastle.jar [*]\n");
    LOGI("  6 0x6fbca100  [79185be08000, 79185bf30000)  r--  /apex/com.android.art/javalib/apache-xml.jar [*]\n");
    LOGI("  7 0x701332b8  [79185b4dc000, 79185be08000)  r--  /system/framework/framework.jar [*]\n");
    LOGI("  8 0x70133328  [79185aba9000, 79185b4dc000)  r--  /system/framework/framework.jar!classes2.dex [*]\n");
    LOGI("  9 0x70133398  [79185a2b7000, 79185aba9000)  r--  /system/framework/framework.jar!classes3.dex [*]\n");
    LOGI(" 10 0x70133408  [79185a193000, 79185a2b7000)  r--  /system/framework/framework.jar!classes4.dex [*]\n");
    LOGI(" ...\n");
    LOGI(" 23 0x71c550b0  [791af11b1000, 791af11bf000)  r--  /apex/com.android.tethering/javalib/framework-tethering.jar [*]\n");
    LOGI(" 24 0x13055198  [791804f7c000, 791805174000)  r--  /data/app/~~Wsw9iRlteEkzqfH0HmhjZA==/penguin.opencore.tester-ATGDVXhbp2-xRwHf7iCsqQ==/base.apk [*]\n");
    LOGI(" 25 0x130b1718  [791848aee000, 791848b21000)  r--  /data/app/~~Wsw9iRlteEkzqfH0HmhjZA==/penguin.opencore.tester-ATGDVXhbp2-xRwHf7iCsqQ==/base.apk!classes2.dex [*]\n");
    LOGI(" 26 0x13050b80  [791aedbfc000, 791aedbff000)  r--  /data/app/~~Wsw9iRlteEkzqfH0HmhjZA==/penguin.opencore.tester-ATGDVXhbp2-xRwHf7iCsqQ==/base.apk!classes3.dex [*]\n");
    ENTER();
    LOGI("core-parser> dex --app\n");
    LOGI("Saved [./base.apk_0xc65c568b].\n");
    LOGI("Saved [./base.apk!classes2.dex_0x9758703].\n");
    LOGI("Saved [./base.apk!classes3.dex_0x4edd148b].\n");
    ENTER();
    LOGI("core-parser> dex -n 7\n");
    LOGI("Saved [./framework.jar_0x347a29fd].\n");
}
