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
#include "command/cmd_auxv.h"
#include "api/core.h"

int AuxvCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return 0;

    auto callback = [](Auxv* auxv) -> bool {
        if (auxv->type() == AT_EXECFN || auxv->type() == AT_PLATFORM) {
            std::string name;
            if (CoreApi::IsVirtualValid(auxv->value())) {
                name = reinterpret_cast<const char*>(CoreApi::GetReal(auxv->value()));
            }
            LOGI("%6" PRIx64 "  %16s  0x%" PRIx64 " " ANSI_COLOR_LIGHTRED "%s\n" ANSI_COLOR_RESET, auxv->type(), auxv->to_string().c_str(),
                                           auxv->value(), name.c_str());
        } else {
            LOGI("%6" PRIx64 "  %16s  0x%" PRIx64 "\n", auxv->type(), auxv->to_string().c_str(), auxv->value());
        }
        return false;
    };
    CoreApi::ForeachAuxv(callback);

    return 0;
}

