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

#ifndef PARSER_COMMAND_FAKE_CORE_LP32_FAKECORE_IMPL_H_
#define PARSER_COMMAND_FAKE_CORE_LP32_FAKECORE_IMPL_H_

#include "command/fake/core/fake_core.h"
#include "lp32/core.h"
#include <linux/elf.h>

namespace lp32 {

class FakeCore : public ::FakeCore {
public:
    FakeCore() : ::FakeCore(),
                 phdr(nullptr), phnum(0),
                 auxv(nullptr), auxvnum(0), use_auxv_num(0),
                 file(nullptr), fileslen(0) {}
    int execute(const char* output);

    void Prepare(const char* filename);
    void ParseProcessMapsVma(std::vector<Opencore::VirtualMemoryArea>& maps);
    void ParserPhdr(int index, Opencore::VirtualMemoryArea& vma);
    void ParserNtFile(int index, Opencore::VirtualMemoryArea& vma);
    void CreateCoreHeader();
    void CreateCoreNoteHeader();
    void CreateCoreAUXV();
    void ClocNoteFileSize();

    uint32_t WriteCoreHeader(std::unique_ptr<MemoryMap>& map, uint32_t off);
    uint32_t WriteCoreNoteHeader(std::unique_ptr<MemoryMap>& map, uint32_t off);
    uint32_t WriteCoreProgramHeaders(std::unique_ptr<MemoryMap>& map, uint32_t off);
    uint32_t WriteCoreAUXV(std::unique_ptr<MemoryMap>& map, uint32_t off);
    uint32_t WriteNtFile(std::unique_ptr<MemoryMap>& map, uint32_t off, std::vector<Opencore::VirtualMemoryArea>& maps);

    static void CreateFakePhdr(uint32_t fake_phdr, uint32_t fake_dynamic);
    static void CreateFakeDynamic(uint32_t fake_dynamic, uint32_t fake_link_map);
    static void CreateFakeLinkMap(uint32_t fake_vma, uint32_t fake_link_map,
                           std::set<std::string>& libs, std::vector<Opencore::VirtualMemoryArea>& maps);
    static void CreateFakeStrtab(uint32_t fake_strtab, uint32_t fake_link_map, std::set<std::string>& libs);

    ~FakeCore();
protected:
    Elf32_Ehdr ehdr;
    Elf32_Phdr *phdr;
    int phnum;
    Elf32_Phdr note;
    Auxv *auxv;
    int auxvnum;
    File *file;
    int fileslen;
    int use_auxv_num;
};

} // namespace lp32

#endif // PARSER_COMMAND_FAKE_CORE_LP32_FAKECORE_IMPL_H_
