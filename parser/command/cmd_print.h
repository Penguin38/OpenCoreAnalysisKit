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

#ifndef PARSER_COMMAND_CMD_PRINT_H_
#define PARSER_COMMAND_CMD_PRINT_H_

#include "command/command.h"
#include "runtime/mirror/object.h"
#include "runtime/mirror/class.h"
#include "runtime/mirror/array.h"
#include "runtime/art_field.h"
#include "android.h"
#include <string>

class PrintCommand : public Command {
public:
    PrintCommand() : Command("print", "p") {}
    ~PrintCommand() {}
    int main(int argc, char* const argv[]);
    void usage();
    void DumpObject(art::mirror::Object& object);
    void DumpClass(art::mirror::Class& clazz);
    void DumpArray(art::mirror::Array& array);
    void DumpInstance(art::mirror::Object& object);
    void PrintField(const char* format, art::mirror::Class& clazz,
                    art::mirror::Object& object, art::ArtField& field);
    std::string FormatSize(uint64_t size);
    void PrintArrayElement(uint32_t i, Android::BasicType type, api::MemoryRef& ref);
    bool PrintReference(art::mirror::Object& object, art::mirror::Object& reference, int cur_deep);

private:
    bool binary;
    bool reference;
    int deep;
};

#endif // PARSER_COMMAND_CMD_PRINT_H_
