/*
 * Copyright (C) 2026-present, Guanyou.Chen. All rights reserved.
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
#include "api/core.h"
#include "command/core/cmd_ptype.h"

int PtypeCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !(argc > 1))
        return Command::FINISH;

    return Command::ONCHLD;
}

int PtypeCommand::main(int argc, char* const argv[]) {
    DumpStructInfo(options, argv[1]);
    return 0;
}

int PtypeCommand::DumpStructInfo(PtypeCommand::Options& options, const char* name) {
    std::string target = name;
    auto callback = [&](LinkMap* map) -> bool {
        std::unique_ptr<dwarf::DwarfLoader>& loader = map->GetDwarfLoader();
        if (!loader)
            return false;

        loader->ForEachStruct([&](const dwarf::StructInfo& si) {
            if (target.size() >= 2 && target.front() == '\'' && target.back() == '\'')
                target = target.substr(1, target.size() - 2);

            if (si.name != target)
                return false;

            if (si.has_size) {
                LOGI("LIB: " ANSI_COLOR_GREEN "%s\n" ANSI_COLOR_RESET, map->name());
                LOGI("[%s]  size=%u\n", si.name.c_str(), si.byte_size);

                for (const auto& bi : si.bases) {
                    if (bi.has_offset)
                        LOGI("    base: %s @ %u\n", bi.type_name.c_str(), bi.offset);
                    else
                        LOGI("    base: %s\n", bi.type_name.c_str());
                }
                for (const auto& mi : si.members) {
                    if (mi.has_offset)
                        LOGI("    [%u] %s : %s\n", mi.offset, mi.name.c_str(), mi.type_name.c_str());
                    else
                        LOGI("    %s : %s\n", mi.name.c_str(), mi.type_name.c_str());
                }
                ENTER();
                return true;
            }
            return false;
        });
        return false;
    };
    CoreApi::ForeachLinkMap(callback);
    return 0;
}

void PtypeCommand::usage() {
    LOGI("Usage: ptype <'art::Runtime'>\n");
}
