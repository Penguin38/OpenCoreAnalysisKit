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

#include "base/memory_map.h"
#include "base/utils.h"
#include <windows.h>
#undef THIS
#include <string.h>

MemoryMap* MemoryMap::MmapFile(const char* file) {
    return MmapFile(file, 0);
}

MemoryMap* MemoryMap::MmapFile(const char* file, uint64_t off) {
    HANDLE hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return nullptr;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return nullptr;
    }

    CloseHandle(hFile);

    if (off >= (uint64_t)fileSize.QuadPart)
        return nullptr;

    return MmapFile(file, fileSize.QuadPart - off, off);
}

MemoryMap* MemoryMap::MmapFile(const char* file, uint64_t size, uint64_t off) {
    HANDLE hFile = CreateFileA(file, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return nullptr;

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        CloseHandle(hFile);
        return nullptr;
    }

    if (off >= (uint64_t)fileSize.QuadPart) {
        CloseHandle(hFile);
        return nullptr;
    }

    HANDLE hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMap) {
        CloseHandle(hFile);
        return nullptr;
    }

    void* mem = MapViewOfFile(hMap, FILE_MAP_READ,
                              (DWORD)(off >> 32), (DWORD)(off & 0xFFFFFFFF), size);
    CloseHandle(hMap);
    CloseHandle(hFile);

    if (!mem)
        return nullptr;

    uint64_t real_size = size;
    if (off + size > (uint64_t)fileSize.QuadPart)
        real_size = fileSize.QuadPart - off;

    MemoryMap* map = new MemoryMap(mem, size, off, real_size);
    map->setFile(file, off);
    return map;
}

MemoryMap* MemoryMap::MmapMem(uint64_t addr, uint64_t size) {
    return MmapMem(addr, size, size);
}

MemoryMap* MemoryMap::MmapMem(uint64_t addr, uint64_t size, uint64_t realSize) {
    void* mem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem)
        return nullptr;

    MemoryMap* map = new MemoryMap(mem, size, 0, size);
    memcpy(mem, reinterpret_cast<void*>(addr), realSize);
    return map;
}

MemoryMap* MemoryMap::MmapZeroMem(uint64_t size) {
    void* mem = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!mem)
        return nullptr;

    MemoryMap* map = new MemoryMap(mem, size, 0, size);
    memset(mem, 0x0, size);
    return map;
}

void MemoryMap::setFile(const char* file, uint64_t off) {
    if (file)
        mName = file;
    mOffset = off;
}

uint32_t MemoryMap::GetCRC32() {
    return Utils::CRC32(reinterpret_cast<uint8_t*>(mBegin), mSize);
}

MemoryMap::~MemoryMap() {
    if (mBegin) {
        // Try UnmapViewOfFile first (for file mappings), fall back to VirtualFree
        if (!UnmapViewOfFile(mBegin))
            VirtualFree(mBegin, 0, MEM_RELEASE);
    }
}
