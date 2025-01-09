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

#ifndef PARSER_COMMAND_FAKE_CORE_FAKECORE_H_
#define PARSER_COMMAND_FAKE_CORE_FAKECORE_H_

#include "command/remote/opencore/opencore.h"
#include "base/macros.h"
#include <set>

/*
             ---------- <-
            |          |  \
          --| Program  |   \
         |  | 1 Header |   |
         |  |          |   |
         |   ----------    |
         |  |          |   |        ----------
       --|--| Program  |   |       |          |
      |  |  | 2 Header |   |       |ELF Header|
      |  |  |          |   \       |          |
      |  |   ----------     ------- ----------       ----> ----------
      |  |  |**********|           |          |     |     |          |
      |  |  |**********|           |          |     |     | Thread 1 |
      |  |  |**********|           |  Program |     |     | Registers|
      |  |  |**********|           |  Headers |     |     |          |
      |  |  |**********|           |          |     |      ----------
      |  |  |**********|           |          |    /      |          | AT_PHDR
      |  |  |**********|           |          |   /       |   FAKE   | -------
      |  |   ----------            |          |  /        |   AUXV   |       |
      |  |  |          |     ------ ---------- --         |          |       |
    --|--|--| Program  |    /      |          |    ------> ----------        |
   |  |  |  | N Header |   |     ->|  Segment |    |                         |
   |  |  |  |          |   /    |  | (PT_NOTE)|    |                         |
   |  |  |   ---------- <--   --   |          |    |                         |
   |  |  |                   |      ---------- ----    --> ----------        |
   |  |   -------------------      |          |       /   |   FAKE   | <------
   |  |                            | FAKE VMA |      /    |   PHDR   |
   |   --------------------------->| (PT_LOAD)|------     -----------
   |                               |          |      \    |   FAKE   |
   |                                ----------        |   | DYNAMIC  |
   |                               |          |       |    ----------
   |                               |  Segment |       |   |   FAKE   |
   |                               | (PT_LOAD)|       |   | LINKMAP  |
   |                               |          |       |    ----------
   |                                ----------        |   |   FAKE   |
   |                               |**********|       |   |  STRTAB  |
   |                               |**********|       ---> ----------
   |                               |**********|
   |                               |**********|
   |                               |**********|
   |                                ----------
   |                               |          |
    ------------------------------>|  Segment |
                                   | (PT_LOAD)|
                                   |          |
                                    ----------
*/

class FakeCore {
public:
    static inline const char* FILE_EXTENSIONS = ".fakecore";
    static inline const char* FAKECORE_VMA = "[FAKECORE]";
    static constexpr int DEF_FAKE_AUXV_NUM = 32;
    static constexpr int DEF_FAKE_PHNUM = 2;
    static constexpr int DEF_FAKE_PRNUM = 1;
    static constexpr int FAKE_PHDR_PAGES = 1;
    static constexpr int FKAE_DYNAMIC_PAGES = 1;
    static constexpr int FAKE_LINK_MAP_PAGES = 1;
    static constexpr int FAKE_STRTAB_PAGES = 1;
    class Stream {
    public:
        virtual ~Stream() {}
        virtual std::string ABI() { return "none"; }
        virtual bool Parse() { return false; }
        virtual int Tid() = 0;
        virtual std::set<std::string>& Libs() = 0;
        virtual std::map<uint64_t, uint64_t>& Memorys() = 0;
        virtual std::vector<Opencore::VirtualMemoryArea>& Maps() = 0;
        virtual void* Regs() = 0;
        virtual uint64_t TaggedAddr() = 0;
        virtual uint64_t PacEnabledKeys() = 0;
    };
    FakeCore() {
        current_offset = 0;
        extra_note_filesz = 0;
        align_size = ELF_PAGE_SIZE;
        page_size = ELF_PAGE_SIZE;
        va_bits = 0;
    }
    void InitStream(std::unique_ptr<FakeCore::Stream>& in) { stream = std::move(in); }
    void InitSysRoot(const char* root) { if (root) sysroot = root; }
    void InitVaBits(int va) { if (va) va_bits = va; }
    void InitPageSize(uint64_t size) { if (IS_ALIGNED(size, ELF_PAGE_SIZE)) page_size = size; }

    virtual ~FakeCore() {}
    virtual int execute(const char* output) { return 0; }
    virtual int getMachine() { return EM_NONE; }

    static int OptionCore(int argc, char* const argv[]);
    static void Usage();
    static std::unique_ptr<FakeCore> Make(int bits);
    static std::unique_ptr<FakeCore> Make(std::unique_ptr<FakeCore::Stream>& stream, const char* sysroot);
    static uint64_t FindModuleLoad(std::vector<Opencore::VirtualMemoryArea>& maps, const char* name);

    std::unique_ptr<FakeCore::Stream>& GetInputStream() { return stream; }
    std::string& GetSysRootDir() { return sysroot; }
protected:
    uint64_t current_offset;
    int extra_note_filesz;
    uint32_t align_size;
    uint32_t page_size;
    int va_bits;
private:
    std::unique_ptr<FakeCore::Stream> stream;
    std::string sysroot;
};

#endif // PARSER_COMMAND_FAKE_CORE_FAKECORE_H_
