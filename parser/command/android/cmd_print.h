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

#ifndef PARSER_COMMAND_ANDROID_CMD_PRINT_H_
#define PARSER_COMMAND_ANDROID_CMD_PRINT_H_

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
    bool prepare(int argc, char* const argv[]);
    void usage();
    void DumpObject(art::mirror::Object& object);
    static void DumpClass(art::mirror::Class& clazz, bool format_hex);
    static void DumpArray(art::mirror::Array& array, bool format_hex);
    static void DumpInstance(art::mirror::Object& object, bool format_hex);
    bool PrintReference(art::mirror::Object& object, art::mirror::Object& reference, int cur_deep);
    static void PrintField(const char* format, art::mirror::Class& clazz,
                    art::mirror::Object& object, art::ArtField& field, bool format_hex);
    static std::string FormatSize(uint64_t size);
    static void PrintArrayElement(uint32_t i, Android::BasicType type, api::MemoryRef& ref, bool format_hex);
    static void OnlyDumpObject(art::mirror::Object& object, bool format_hex);
private:
    bool binary;
    bool reference;
    bool format_dump;
    bool format_hex;
    int deep;
};

#endif // PARSER_COMMAND_ANDROID_CMD_PRINT_H_
