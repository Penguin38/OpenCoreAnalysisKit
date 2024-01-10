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

#ifndef CORE_API_CORE_H_
#define CORE_API_CORE_H_

#include "api/thread.h"
#include "base/memory_map.h"
#include "common/load_block.h"
#include "common/note_block.h"
#include "common/link_map.h"
#include "common/file.h"
#include <stdint.h>
#include <sys/types.h>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <map>

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
      |  |  |**********|           |          |    /      |          |
      |  |  |**********|           |          |   /       | Thread 2 |
      |  |   ----------            |          |  /        | Registers|
      |  |  |          |     ------ ---------- --         |          |
    --|--|--| Program  |    /      |          |            ----------
   |  |  |  | N Header |   |     ->|  Segment |           |**********|
   |  |  |  |          |   /    |  | (PT_NOTE)|           |**********|
   |  |  |   ---------- <--   --   |          |           |**********|
   |  |  |                   |      ---------- --         |**********|
   |  |   -------------------      |          |  \         ----------
   |  |                            |  Segment |   \       |          |
   |   --------------------------->| (PT_LOAD)|    |      | Thread N |
   |                               |          |    |      | Registers|
   |                                ----------     |      |          |
   |                               |          |    |       ----------
   |                               |  Segment |    \      |          |
   |                               | (PT_LOAD)|     \     |   AUXV   |
   |                               |          |      \    |          |
   |                                ----------        ---> ----------
   |                               |**********|
   |                               |**********|
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

static constexpr int OPT_READ_OR = LoadBlock::OPT_READ_OR;
static constexpr int OPT_READ_MMAP = LoadBlock::OPT_READ_MMAP;
static constexpr int OPT_READ_OVERLAY = LoadBlock::OPT_READ_OVERLAY;
static constexpr int OPT_READ_ALL = LoadBlock::OPT_READ_ALL;

class CoreApi {
public:
    static bool Load(const char* corefile);
    static void UnLoad();
    static const char* GetMachineName();
    static int GetMachine();
    static int GetPointSize();
    static uint64_t GetReal(uint64_t vaddr) {
        return GetReal(vaddr, OPT_READ_ALL);
    }
    static uint64_t GetReal(uint64_t vaddr, int opt);
    static uint64_t GetVirtual(uint64_t raddr);
    static bool IsVirtualValid(uint64_t vaddr);
    static uint64_t FindAuxv(uint64_t type);
    static ThreadApi* FindThread(int tid);
    static uint64_t GetDebug();
    // Command
    static void DumpFile();
    static void DumpLinkMap();
    static void ExecFile(const char* file);
    static void SysRoot(const char* dir);
    static void Write(uint64_t vaddr, uint64_t value);
    static bool Read(uint64_t vaddr, uint64_t size, uint8_t* buf) {
        return Read(vaddr, size, buf, OPT_READ_ALL);
    }
    static bool Read(uint64_t vaddr, uint64_t size, uint8_t* buf, int opt);

    CoreApi() {}
    CoreApi(std::unique_ptr<MemoryMap>& map) {
        mCore = std::move(map);
    }
    virtual ~CoreApi();
    uint64_t begin();
    uint64_t size();
    std::string& getName();
    void addLoadBlock(std::shared_ptr<LoadBlock>& block);
    LoadBlock* findLoadBlock(uint64_t vaddr);
    void removeAllLoadBlock();
    inline uint64_t v2r(uint64_t vaddr, int opt);
    inline uint64_t r2v(uint64_t raddr);
    inline bool virtualValid(uint64_t vaddr);
    void addNoteBlock(std::unique_ptr<NoteBlock>& block);
    void removeAllNoteBlock();
    uint64_t findAuxv(uint64_t type);
    ThreadApi* findThread(int tid);
    void setDebug(uint64_t debug) { mDebug = debug; }
    void addLinkMap(uint64_t begin, uint64_t name);
    void removeAllLinkMap();
    void foreachFile(std::function<void (File *)> callback);
    void foreachLinkMap(std::function<void (LinkMap *)> callback);
private:
    static CoreApi* INSTANCE;
    virtual bool load() = 0;
    virtual void unload() = 0;
    virtual const char* getMachineName() = 0;
    virtual int getMachine() = 0;
    virtual int getPointSize() = 0;
    virtual void loadDebug() = 0;
    virtual void loadLinkMap() = 0;
    virtual void sysroot(uint64_t begin, const char* file) = 0;

    std::unique_ptr<MemoryMap> mCore;
    std::vector<std::shared_ptr<LoadBlock>> mLoad;
    std::vector<std::unique_ptr<NoteBlock>> mNote;
    std::vector<std::unique_ptr<LinkMap>> mLinkMap;
protected:
    uint64_t mDebug;
};

#endif // CORE_API_CORE_H_
