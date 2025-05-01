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
#include "common/exception.h"
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
    static constexpr uint64_t FAKE_LOAD_BEGIN = 0x100000;

    static bool IsReady();
    static bool Load(const char* corefile, std::function<void ()> callback);
    static bool Load(const char* corefile, bool remote, std::function<void ()> callback);
    static bool Load(std::unique_ptr<MemoryMap>& map, bool remote, std::function<void ()> callback);
    static void UnLoad() { INSTANCE.reset(); }
    static uint64_t GetBegin() { return INSTANCE->begin(); }
    static uint64_t GetDebugPtr() { return INSTANCE->r_debug_ptr().Ptr(); }
    static const char* GetName();
    static const char* GetMachineName();
    static int GetMachine();
    static int Bits() { return INSTANCE->bits(); }
    static int GetPointSize();
    static uint64_t GetPointMask();
    static uint64_t GetVabitsMask();
    static uint64_t GetPageSize() { return INSTANCE->getPageSize(); }
    static bool IsRemote() { return IsReady() && INSTANCE->isRemote(); }
    static std::vector<std::shared_ptr<LoadBlock>>& GetLoads(bool quick) {
        return INSTANCE->getLoads(quick);
    }
    static std::vector<std::unique_ptr<NoteBlock>>& GetNotes() {
        return INSTANCE->getNotes();
    }
    static std::vector<std::unique_ptr<LinkMap>>& GetLinkMaps() {
        return INSTANCE->mLinkMap;
    }
    static uint64_t GetReal(uint64_t vaddr) {
        return GetReal(vaddr, OPT_READ_ALL);
    }
    static uint64_t GetReal(uint64_t vaddr, int opt);
    static uint64_t GetVirtual(uint64_t raddr);
    static bool IsVirtualValid(uint64_t vaddr);
    static uint64_t FindAuxv(uint64_t type);
    static ThreadApi* FindThread(int tid);
    static void Init();
    static void Dump();
    static void CleanCache();
    static void ForeachFile(std::function<bool (File *)> callback);
    static void ForeachAuxv(std::function<bool (Auxv *)> callback);
    static void ForeachLinkMap(std::function<bool (LinkMap *)> callback);
    static File* FindFile(uint64_t vaddr);
    static LinkMap* FindLinkMap(const char* path);
    static void ExecFile(const char* file);
    static void SysRoot(const char* dir);
    static void Write(uint64_t vaddr, uint64_t value) {
        Write(vaddr, &value, 8);
    }
    static void Write(uint64_t vaddr, void *buf, uint64_t size);
    static bool Read(uint64_t vaddr, uint64_t size, uint8_t* buf) {
        return Read(vaddr, size, buf, OPT_READ_ALL);
    }
    static bool Read(uint64_t vaddr, uint64_t size, uint8_t* buf, int opt);

    // default non-quick search load
    static void ForeachLoadBlock(std::function<bool (LoadBlock *)> callback) {
        return ForeachLoadBlock(callback, true /** filter invalid */);
    }
    static void ForeachLoadBlock(std::function<bool (LoadBlock *)> callback, bool check) {
        return ForeachLoadBlock(callback, check, false /** mLoad */);
    }
    static void ForeachLoadBlock(std::function<bool (LoadBlock *)> callback, bool check, bool quick);

    // default quick search load
    static inline LoadBlock* FindLoadBlock(uint64_t vaddr) {
        return FindLoadBlock(vaddr, true /** throw invalid */);
    }
    static inline LoadBlock* FindLoadBlock(uint64_t vaddr, bool check) {
        return FindLoadBlock(vaddr, check, true /** mQuickLoad */);
    }
    static inline LoadBlock* FindLoadBlock(uint64_t vaddr, bool check, bool quick) {
        LoadBlock* block = INSTANCE->findLoadBlock(vaddr, quick);
        if (check) {
            if (block && block->isValid())
                return block;
            throw InvalidAddressException(vaddr);
        }
        return block;
    }
    static uint64_t DlSym(const char* symbol);
    static uint64_t DlSym(const char* path, const char* symbol);
    static void ForeachThread(std::function<bool (ThreadApi *)> callback);
    static uint64_t NewLoadBlock(uint64_t size) {
        return NewLoadBlock(0x0, size);
    }
    static uint64_t NewLoadBlock(uint64_t vaddr, uint64_t size) {
        return NewLoadBlock(vaddr, size, Block::FLAG_R | Block::FLAG_W);
    }
    static uint64_t NewLoadBlock(uint64_t vaddr, uint64_t size, int flags);
    static void RegisterSysRootListener(std::function<void (LinkMap *)> fn) {
        INSTANCE->mSysRootCallback = fn;
    }

    CoreApi(std::unique_ptr<MemoryMap>& map)
            : pointer_mask(-1),
              vabits_mask(-1),
              page_size(0) {
        mCore = std::move(map);
    }
    virtual ~CoreApi();
    uint64_t begin();
    uint64_t size();
    std::string& getName();
    void addLoadBlock(std::shared_ptr<LoadBlock>& block);

    /*
     * strongly increasing sort
     * M0 S1   E1M1 S2M2 E2M3...
     *  | *----*  | *-|--* | ...
     *  M0 S1 < E1 <= M1 < S2 <= M2 < E2 <= M3 ...
     */
    inline LoadBlock* findLoadBlock(uint64_t vaddr, bool quick) {
        std::vector<std::shared_ptr<LoadBlock>>& loads = getLoads(quick);
        if (loads.empty()) return nullptr;

        int left = 0;
        int right = loads.size();
        uint64_t clocaddr = vaddr & getVabitsMask();

        if (clocaddr < loads[left]->vaddr()
                || clocaddr >= (loads[right - 1]->vaddr() + loads[right - 1]->memsz()))
           return nullptr;

        while (left < right) {
            int mid = left + (right - left) / 2;
            LoadBlock* block = loads[mid].get();
            if (block->vaddr() > clocaddr) {
                right = mid;
            } else {
                if (clocaddr < (block->vaddr() + block->memsz()))
                    return block;
                left = mid + 1;
            }
        }
        return nullptr;
    }
    void removeAllLoadBlock();
    void removeAllBindMap();
    inline uint64_t v2r(uint64_t vaddr, int opt);
    inline uint64_t r2v(uint64_t raddr);
    inline bool virtualValid(uint64_t vaddr);
    void addNoteBlock(std::unique_ptr<NoteBlock>& block);
    void removeAllNoteBlock();
    uint64_t findAuxv(uint64_t type);
    ThreadApi* findThread(int tid);
    void addLinkMap(uint64_t map);
    void removeAllLinkMap();
    void foreachThread(std::function<bool (ThreadApi *)> callback);
    void foreachFile(std::function<bool (File *)> callback);
    void foreachAuxv(std::function<bool (Auxv *)> callback);
    void foreachLinkMap(std::function<bool (LinkMap *)> callback);
    void foreachLoadBlock(std::function<bool (LoadBlock *)> callback, bool check, bool quick);
    uint64_t newLoadBlock(uint64_t vaddr, uint64_t size, int flags);
    uint64_t getPageSize();
    inline std::vector<std::shared_ptr<LoadBlock>>& getLoads(bool quick) {
        return quick? mQuickLoad : mLoad;
    }
    inline std::vector<std::unique_ptr<NoteBlock>>& getNotes() {
        return mNote;
    }
    bool isRemote() { return mRemote; }
    static bool QUICK_LOAD_ENABLED;
    static uint64_t VA_BITS;
protected:
    uint64_t pointer_mask;
    uint64_t vabits_mask;
    uint64_t page_size;
private:
    static std::unique_ptr<CoreApi> INSTANCE;
    virtual bool load() = 0;
    virtual void unload() = 0;
    virtual const char* getMachineName() = 0;
    virtual int bits() = 0;
    virtual int getMachine() = 0;
    virtual int getPointSize() = 0;
    virtual uint64_t getVabitsMask() = 0;
    virtual void loadLinkMap() = 0;
    virtual bool exec(uint64_t phdr, const char* file) = 0;
    virtual bool sysroot(LinkMap* handle, const char* file, const char* subfile) = 0;
    virtual api::MemoryRef& r_debug_ptr() = 0;

    std::unique_ptr<MemoryMap> mCore;
    std::vector<std::shared_ptr<LoadBlock>> mLoad;
    std::vector<std::shared_ptr<LoadBlock>> mQuickLoad;
    std::vector<std::unique_ptr<NoteBlock>> mNote;
    std::vector<std::unique_ptr<LinkMap>> mLinkMap;
    std::function<void (LinkMap *)> mSysRootCallback;
    bool mRemote = false;
};

#endif // CORE_API_CORE_H_
