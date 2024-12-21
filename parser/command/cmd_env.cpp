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
#include "android.h"
#include "runtime/cache_helpers.h"
#include "command/cmd_env.h"
#include "command/env.h"
#include "api/core.h"
#include "api/elf.h"
#include "common/elf.h"
#include "common/disassemble/capstone.h"
#include "base/utils.h"
#include "base/macros.h"
#include <linux/elf.h>
#include <unistd.h>
#include <getopt.h>

typedef int (*EnvCall)(int argc, char* const argv[]);
struct EnvOption {
    const char* cmd;
    EnvCall call;
};

static EnvOption env_option[] = {
    { "config", EnvCommand::onConfigChanged },
    { "logger", EnvCommand::onLoggerChanged },
    { "art", EnvCommand::showArtEnv },
    { "core", EnvCommand::showCoreEnv },
    { "offset", EnvCommand::onOffsetChanged },
    { "size", EnvCommand::onSizeChanged },
};

int EnvCommand::main(int argc, char* const argv[]) {
    if (!(argc > 1))
        return dumpEnv();

    int count = sizeof(env_option)/sizeof(env_option[0]);
    for (int index = 0; index < count; ++index) {
        if (!strcmp(argv[1], env_option[index].cmd)) {
            return env_option[index].call(argc - 1, &argv[1]);
        }
    }

    LOGI("unknown command (%s)\n", argv[1]);
    return 0;
}

int EnvCommand::onConfigChanged(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    int opt;
    int option_index = 0;
    int current_sdk = Android::V;
    int current_pid = Env::CurrentPid();
    int current_oat = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"pid",     required_argument, 0, 'p'},
        {"sdk",     required_argument, 0,  0 },
        {"oat",     required_argument, 0,  1 },
        {0,         0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "p:0:1:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                current_pid = std::atoi(optarg);
                if (Env::SetCurrentPid(current_pid))
                    Env::Dump();
                break;
            case 0:
                if (Android::IsReady()) {
                    current_sdk = std::atoi(optarg);
                    Android::OnSdkChanged(current_sdk);
                }
                break;
            case 1:
                if (Android::IsReady()) {
                    current_oat = std::atoi(optarg);
                    Android::OnOatChanged(current_oat);
                }
                break;
        }
    }

    return 0;
}

int EnvCommand::onLoggerChanged(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"debug",   required_argument, 0, 'd'},
        {"info",    no_argument,       0, Logger::LEVEL_INFO},
        {"warn",    no_argument,       0, Logger::LEVEL_WARN},
        {"error",   no_argument,       0, Logger::LEVEL_ERROR},
        {"fatal",   no_argument,       0, Logger::LEVEL_FATAL},
        {"enable-high-light",  no_argument, 0,  11 },
        {"disable-high-light", no_argument, 0,  12 },
    };

    if (argc < 2) {
        LOGI("Logger level %s\n", long_options[Logger::GetLevel()].name);
        LOGI("Logger debug level: %d\n", Logger::GetDebugLevel());
        LOGI("Logger high-light: %s\n", Logger::IsLight() ? "true" : "false");
        return 0;
    }

    while ((opt = getopt_long(argc, argv, "d:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd': {
                int lv = std::atoi(optarg);
                if (lv >= Logger::LEVEL_NONE && lv <= Logger::LEVEL_DEBUG_2) {
                    LOGI("Switch logger debug level: %d\n", lv);
                    Logger::SetDebugLevel(lv);
                }
            } break;
            case 1:
            case 2:
            case 3:
            case 4:
                LOGI("Switch logger level %s\n", long_options[option_index].name);
                Logger::SetLevel(opt);
                break;
            case 11:
                Logger::SetHighLight(true);
                break;
            case 12:
                Logger::SetHighLight(false);
                break;
        }
    }
    return 0;
}

int EnvCommand::showArtEnv(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsReady())
        return 0;

    art::Runtime& runtime = Android::GetRuntime();

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"clean-cache",   no_argument, 0, 'c'},
        {"entry-points",  no_argument, 0, 'e'},
        {"nterp",         no_argument, 0, 'n'},
    };

    while ((opt = getopt_long(argc, argv, "cen",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                Android::Reset();
                return 0;
            case 'e':
                art::CacheHelper::EntryPointDump();
                return 0;
            case 'n':
                art::CacheHelper::NterpDump();
                return 0;
        }
    }

    LOGI("  * LIB: " ANSI_COLOR_LIGHTGREEN "%s\n" ANSI_COLOR_RESET, Android::GetRealLibart().c_str());
    LOGI("  * art::OatHeader::kOatVersion: " ANSI_COLOR_LIGHTMAGENTA "%d\n" ANSI_COLOR_RESET, Android::Oat());
    LOGI("  * art::Runtime: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.Ptr());
    if (art::Runtime::Origin() != runtime.Ptr())
        LOGI("  * art::Runtime::instance_: " ANSI_COLOR_LIGHTRED "0x%lx\n" ANSI_COLOR_RESET, art::Runtime::Origin().Ptr());

    if (!runtime.Ptr())
        return 0;

    LOGI("  * art::gc::Heap: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetHeap().Ptr());
    if (runtime.GetHeap().Ptr()) {
        LOGI("  *     continuous_spaces_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetHeap().GetContinuousSpacesCache().Ptr());
        LOGI("  *     discontinuous_spaces_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetHeap().GetDiscontinuousSpacesCache().Ptr());
    }
    LOGI("  * art::MonitorPool: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetMonitorPool().Ptr());
    LOGI("  * art::ThreadList: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetThreadList().Ptr());
    if (runtime.GetThreadList().Ptr()) {
        LOGI("  *     list_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetThreadList().GetListCache().Ptr());
    }
    LOGI("  * art::ClassLinker: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetClassLinker().Ptr());
    if (runtime.GetClassLinker().Ptr()) {
        if (Android::Sdk() < Android::T) {
            LOGI("  *     dex_caches_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetClassLinker().GetDexCachesData().Ptr());
        } else {
            LOGI("  *     dex_caches_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetClassLinker().GetDexCachesData_v33().Ptr());
        }
    }
    LOGI("  * art::JavaVMExt: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetJavaVM().Ptr());
    if (runtime.GetJavaVM().Ptr()) {
        LOGI("  *     globals_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetJavaVM().GetGlobalsTable().Ptr());
        LOGI("  *     weak_globals_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetJavaVM().GetWeakGlobalsTable().Ptr());
    }
    LOGI("  * art::jit::Jit: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetJit().Ptr());
    if (runtime.GetJit().Ptr()) {
        LOGI("  *     code_cache_: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, runtime.GetJit().GetCodeCache().Ptr());
    }
    return 0;
}

int EnvCommand::showCoreEnv(int argc, char* const argv[]) {
    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"load",   no_argument,       0, 1},
        {"arm",    required_argument, 0, 2},
        {"crc",    no_argument,       0, 3},
        {"num",    required_argument, 0,'n'},
        {"quick-load", no_argument,   0, 4},
        {"note",   no_argument,       0, 5},
        {"clean-cache",   no_argument, 0, 'c'},
    };

    bool crc = false;
    int num = 0;
    while ((opt = getopt_long(argc, argv, "12:3:4n:c",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 1: return showLoadEnv(false);
            case 2:
                capstone::Disassember::SetArmMode(optarg);
                return 0;
            case 3:
                crc = true;
                break;
            case 4: return showLoadEnv(true);
            case 5: return showNoteEnv();
            case 'n':
                num = std::atoi(optarg);
                break;
            case 'c':
                CoreApi::CleanCache();
                return 0;
        }
    }
    if (crc) {
        clocLoadCRC32(num);
    } else {
        LOGI("  * r_debug: " ANSI_COLOR_LIGHTMAGENTA "0x%lx\n" ANSI_COLOR_RESET, CoreApi::GetDebugPtr());
        LOGI("  * arm mode: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, !capstone::Disassember::GetArmMode() ? "arm" : "thumb");
        LOGI("  * mNote: " ANSI_COLOR_LIGHTMAGENTA "%ld\n" ANSI_COLOR_RESET, CoreApi::GetNotes().size());
        LOGI("  * mLoad: " ANSI_COLOR_LIGHTMAGENTA "%ld\n" ANSI_COLOR_RESET, CoreApi::GetLoads(false).size());
        LOGI("  * mQuickLoad: " ANSI_COLOR_LIGHTMAGENTA "%ld\n" ANSI_COLOR_RESET, CoreApi::GetLoads(true).size());
        LOGI("  * mLinkMap: " ANSI_COLOR_LIGHTMAGENTA "%ld\n" ANSI_COLOR_RESET, CoreApi::GetLinkMaps().size());
    }
    return 0;
}

int EnvCommand::showLoadEnv(bool quick) {
    if (!CoreApi::IsReady())
        return 0;

    int index = 0;
    auto callback = [&index](LoadBlock *block) -> bool {
        index++;
        std::string name;
        if (block->name().length() > 0) {
            name.append(Logger::Green());
            name.append(block->name());
            name.append(Logger::End());
        } else {
            name.append("[]");
        }
        LOGI("  %-5d " ANSI_COLOR_CYAN "[%lx, %lx)" ANSI_COLOR_RESET "  %s  %010lx  ""%s"" %s\n",
                index, block->vaddr(), block->vaddr() + block->size(), block->convertFlags().c_str(),
                block->realSize(), name.c_str(), block->convertValids().c_str());
        return false;
    };
    LOGI(ANSI_COLOR_LIGHTRED "INDEX   REGION               FLAGS FILESZ      PATH\n" ANSI_COLOR_RESET);
    CoreApi::ForeachLoadBlock(callback, false, quick);
    return 0;
}

int EnvCommand::showNoteEnv() {
    if (!CoreApi::IsReady())
        return 0;

    int index = 0;
    LOGI(ANSI_COLOR_LIGHTRED "INDEX   OFFSET        FILESZ\n" ANSI_COLOR_RESET);
    for (const auto& note : CoreApi::GetNotes()) {
        LOGI("  %-5d 0x%08lx    0x%08lx [*]%s\n", index,
                note->offset(), note->realSize(), note->isOverlayBlock()? "(OVERLAY)" : "");
        ++index;
    }
    return 0;
}

int EnvCommand::clocLoadCRC32(int num) {
    bool cloc_all = num == 0;
    if (!CoreApi::IsReady())
        return 0;

    bool first = true;
    int index = 0;
    auto callback = [&](LoadBlock *block) -> bool {
        index++;
        if (!cloc_all && num != index)
            return false;

        if (!block->isMmapBlock() || !block->isValidBlock())
            return false;

        // only check .text crc32
        // if (!(block->flags() & Block::FLAG_X))
        //    return false;

        if (cloc_all || num == index) {
            uint32_t or_crc = 0x0;
            uint32_t mmap_crc = 0x0;

            ElfHeader* header = reinterpret_cast<ElfHeader*>(block->begin(LoadBlock::OPT_READ_MMAP));
            if (!memcmp(header->ident, ELFMAG, 4)) {
                // skip elf header
                or_crc = Utils::CRC32(reinterpret_cast<uint8_t*>(block->begin(LoadBlock::OPT_READ_OR)) + SIZEOF(Elfx_Ehdr),
                        block->size() - SIZEOF(Elfx_Ehdr));
                mmap_crc = Utils::CRC32(reinterpret_cast<uint8_t*>(block->begin(LoadBlock::OPT_READ_MMAP)) + SIZEOF(Elfx_Ehdr),
                        block->size() - SIZEOF(Elfx_Ehdr));
            } else {
                or_crc = block->GetCRC32(LoadBlock::OPT_READ_OR);
                mmap_crc = block->GetCRC32(LoadBlock::OPT_READ_MMAP);
            }

            if (or_crc != mmap_crc) {
                std::string name;
                name.append(Logger::Green());
                name.append(block->name());
                name.append(Logger::End());

                if (!first) { ENTER(); }
                first = false;
                LOGI("%-5d " ANSI_COLOR_CYAN "[%lx, %lx)" ANSI_COLOR_RESET "  %s  %010lx  ""%s""\n",
                        index, block->vaddr(), block->vaddr() + block->size(), block->convertFlags().c_str(),
                        block->realSize(), name.c_str());

                uint64_t* orv = reinterpret_cast<uint64_t*>(block->begin(LoadBlock::OPT_READ_OR));
                uint64_t* mmv = reinterpret_cast<uint64_t*>(block->begin(LoadBlock::OPT_READ_MMAP));
                int count = RoundUp(block->size() / 8, 2);
                LinkMap::NiceSymbol symbol;
                for (int k = 0; k < count; k += 2) {
                    uint64_t orv1 = orv[k];
                    uint64_t orv2 = orv[k + 1];
                    uint64_t mmv1 = mmv[k];
                    uint64_t mmv2 = mmv[k + 1];
                    if (LIKELY(orv1 == mmv1) && LIKELY(orv2 == mmv2))
                        continue;

                    uint64_t current = block->vaddr() + k * 8;
                    if (!symbol.IsValid() ||
                            (current < symbol.GetOffset() ||
                             current >= symbol.GetOffset() + symbol.GetSize())) {
                        if (block->handle()) {
                            symbol = LinkMap::NiceSymbol::Invalid();
                            block->handle()->NiceMethod(current, symbol);
                            if (symbol.IsValid()) LOGI(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET ":\n", symbol.GetSymbol().c_str());
                        }
                    }
                    LOGI(ANSI_COLOR_CYAN "%lx" ANSI_COLOR_RESET ": %016lx  %016lx  %s%s  |  %016lx  %016lx  %s%s\n",
                            current, orv1, orv2,
                            Utils::ConvertAscii(orv1, 8).c_str(), Utils::ConvertAscii(orv2, 8).c_str(),
                            mmv1, mmv2,
                            Utils::ConvertAscii(mmv1, 8).c_str(), Utils::ConvertAscii(mmv2, 8).c_str());
                }
            }
        }
        return false;
    };
    CoreApi::ForeachLoadBlock(callback, false);
    return 0;
}

int EnvCommand::dumpEnv() {
    if (CoreApi::IsReady()) {
        CoreApi::Dump();
        Env::Dump();

        if (Android::IsReady()) {
            Android::Dump();
        }
    }
    return 0;
}

int EnvCommand::onOffsetChanged(int argc, char* const argv[]) {
    return 0;
}

int EnvCommand::onSizeChanged(int argc, char* const argv[]) {
    return 0;
}

void EnvCommand::usage() {
    LOGI("Usage: env <COMMAND> [OPTION] ...\n");
    LOGI("Command:\n");
    LOGI("    config  logger  art  core\n");
    ENTER();

    LOGI("Usage: env config <OPTION> ..\n");
    LOGI("Option:\n");
    LOGI("        --sdk <VERSION>   set current sdk version\n");
    LOGI("        --oat <VERSION>   set current oat version\n");
    LOGI("    -p, --pid <PID>       set current thread\n");
    ENTER();
    LOGI("core-parser> env config --sdk 30\n");
    LOGI("Switch android(30) env.\n");
    ENTER();

    LOGI("Usage: env logger <OPTION>\n");
    LOGI("Option:\n");
    LOGI("        --debug <LEVEL>   set current logger debug level\n");
    LOGI("        --info            set current logger level to info\n");
    LOGI("        --warn            set current logger level to warn\n");
    LOGI("        --error           set current logger level to error\n");
    LOGI("        --fatal           set current logger level to fatal\n");
    LOGI("    --enable-high-light   enable logger output high-light\n");
    LOGI("    --disable-high-light  disable logger output high-light\n");
    ENTER();
    LOGI("core-parser> env logger\n");
    LOGI("Logger level error\n");
    LOGI("Logger debug level: 0\n");
    LOGI("Logger high-light: true\n");
    ENTER();

    LOGI("Usage: env art [OPTION] ...\n");
    LOGI("Option:\n");
    LOGI("    -c, --clean-cache     clean art::Runtime cache\n");
    LOGI("    -e, --entry-points    show art quick entry points\n");
    LOGI("    -n, --nterp           show art nterp cache\n");
    ENTER();
    LOGI("core-parser> env art\n");
    LOGI("  * LIB: /apex/com.android.art/lib64/libart.so\n");
    LOGI("  * art::OatHeader::kOatVersion: 183\n");
    LOGI("  * art::Runtime: 0x79196ce69360\n");
    LOGI("  * art::gc::Heap: 0x79196ce6d3c0\n");
    LOGI("  *     continuous_spaces_: 0x79196ce6d3c0\n");
    LOGI("  *     discontinuous_spaces_: 0x79196ce6d3d8\n");
    LOGI("  * art::MonitorPool: 0x7918cce64ae0\n");
    LOGI("  * art::ThreadList: 0x7919dce69430\n");
    LOGI("  *     list_: 0x7919dce6b430\n");
    LOGI("  * art::ClassLinker: 0x79192ce6b090\n");
    LOGI("  *     dex_caches_: 0x79192ce6b0c8\n");
    LOGI("  * art::JavaVMExt: 0x79192ce68310\n");
    LOGI("  *     globals_: 0x79192ce68350\n");
    LOGI("  *     weak_globals_: 0x79192ce683d8\n");
    LOGI("  * art::jit::Jit: 0x79193ce73a70\n");
    LOGI("  *     code_cache_: 0x79196ce6ad20\n");
    ENTER();

    LOGI("Usage: env core [OPTION]...\n");
    LOGI("Option:\n");
    LOGI("        --note            show corefile note segments\n");
    LOGI("        --load            show corefile load segments\n");
    LOGI("        --quick-load      show corefile quick load segments\n");
    LOGI("        --arm <thumb|arm> set arm disassemble mode\n");
    LOGI("        --crc             check consistency of mmap file data\n");
    LOGI("    -c, --clean-cache     clean link_map cache\n");
    ENTER();
    LOGI("core-parser> env core\n");
    LOGI("  * r_debug: 0x791af2dd7bf0\n");
    LOGI("  * arm mode: thumb\n");
    LOGI("  * mNote: 1\n");
    LOGI("  * mLoad: 1985\n");
    LOGI("  * mQuickLoad: 1802\n");
    LOGI("  * mLinkMap: 271\n");
}
