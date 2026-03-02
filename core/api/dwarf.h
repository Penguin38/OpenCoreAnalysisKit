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

#ifndef CORE_API_DWARF_H_
#define CORE_API_DWARF_H_

#include <stdint.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class MemoryMap;

namespace dwarf {

// DWARF tag constants
constexpr uint32_t DW_TAG_array_type            = 0x01;
constexpr uint32_t DW_TAG_class_type            = 0x02;
constexpr uint32_t DW_TAG_enumeration_type      = 0x04;
constexpr uint32_t DW_TAG_member                = 0x0d;
constexpr uint32_t DW_TAG_pointer_type          = 0x0f;
constexpr uint32_t DW_TAG_reference_type        = 0x10;
constexpr uint32_t DW_TAG_typedef               = 0x16;
constexpr uint32_t DW_TAG_union_type            = 0x17;
constexpr uint32_t DW_TAG_inheritance           = 0x1c;
constexpr uint32_t DW_TAG_structure_type        = 0x13;
constexpr uint32_t DW_TAG_base_type             = 0x24;
constexpr uint32_t DW_TAG_const_type            = 0x26;
constexpr uint32_t DW_TAG_volatile_type         = 0x35;
constexpr uint32_t DW_TAG_namespace             = 0x39;
constexpr uint32_t DW_TAG_rvalue_reference_type = 0x42;

// DWARF attribute constants
constexpr uint32_t DW_AT_name                 = 0x03;
constexpr uint32_t DW_AT_byte_size            = 0x0b;
constexpr uint32_t DW_AT_data_member_location = 0x38;
constexpr uint32_t DW_AT_type                 = 0x49;
constexpr uint32_t DW_AT_str_offsets_base     = 0x72; // DWARF 5

// DW_OP opcodes
constexpr uint8_t DW_OP_plus_uconst = 0x23;

struct AttrSpec {
    uint32_t name;
    uint32_t form;
    int64_t  implicit_const; // used when form == DW_FORM_implicit_const (0x21)
};

struct AbbrevEntry {
    uint32_t code;
    uint32_t tag;
    bool has_children;
    std::vector<AttrSpec> attrs;
};

using AbbrevTable = std::unordered_map<uint32_t, AbbrevEntry>;
using AbbrevMap   = std::unordered_map<uint64_t, AbbrevTable>;

struct BaseInfo {
    std::string type_name;  // base class full name (e.g., "art::CodeItemDataAccessor")
    uint32_t offset;
    bool has_offset;
};

struct MemberInfo {
    std::string name;
    std::string type_name;  // resolved type name (e.g., "int", "Runtime*")
    uint32_t offset;
    bool has_offset;
};

struct StructInfo {
    std::string name;
    uint32_t byte_size;
    bool has_size;
    std::vector<BaseInfo>   bases;    // base classes (from DW_TAG_inheritance)
    std::vector<MemberInfo> members;
};

class DwarfLoader {
public:
    static std::unique_ptr<DwarfLoader> Load(const char* elf_path);
    void ForEachStruct(std::function<bool (const StructInfo&)> cb) const;

private:
    bool Init(const char* elf_path);
    bool LocateSections();
    void ParseAbbrevSection();
    const uint8_t* ParseAbbrevTable(const uint8_t* ptr, const uint8_t* end,
                                    uint64_t base_offset);
    void ParseInfoSection();
    const uint8_t* ParseCompileUnit(const uint8_t* ptr, const uint8_t* end);
    const uint8_t* ParseDIETree(const uint8_t* ptr, const uint8_t* cu_end,
                                uint32_t addr_size, bool dwarf64,
                                const AbbrevTable& abbrev_table,
                                const std::string& ns_prefix,
                                uint64_t str_offsets_base,
                                uint64_t cu_base_offset);
    const uint8_t* ReadAttrValue(const uint8_t* ptr, const uint8_t* end,
                                 uint32_t form, uint32_t addr_size, bool dwarf64,
                                 uint64_t str_offsets_base,
                                 uint64_t* out_value, std::string* out_str);
    static bool ParseExprloc(const uint8_t* data, uint64_t size, uint64_t* out_value);
    static uint64_t ReadULEB128(const uint8_t** ptr, const uint8_t* end);
    static int64_t  ReadSLEB128(const uint8_t** ptr, const uint8_t* end);
    const char* GetDebugStr(uint64_t offset) const;
    const char* GetDebugStrX(uint64_t index, uint64_t str_offsets_base, bool dwarf64) const;
    bool DecompressSection(const uint8_t* raw, uint64_t raw_size,
                           const uint8_t** out_data, uint64_t* out_size);
    void ResolveTypes();
    std::string ResolveTypeName(uint64_t offset, int depth) const;

    std::unique_ptr<MemoryMap> map_;
    const uint8_t* debug_abbrev_data_        = nullptr; uint64_t debug_abbrev_size_        = 0;
    const uint8_t* debug_info_data_          = nullptr; uint64_t debug_info_size_          = 0;
    const uint8_t* debug_str_data_           = nullptr; uint64_t debug_str_size_           = 0;
    const uint8_t* debug_str_offsets_data_   = nullptr; uint64_t debug_str_offsets_size_   = 0;
    int elf_bits_ = 64;
    AbbrevMap abbrev_map_;
    std::vector<StructInfo> structs_;
    std::vector<std::vector<uint8_t>> decompressed_sections_;

    // Type registry: absolute .debug_info offset → type entry
    struct TypeEntry {
        std::string name;      // empty for unnamed (pointer/const/volatile/array)
        uint64_t    type_ref;  // chained type offset (for pointer/typedef/const/...)
        uint8_t     qualifier; // 0=named, 1=pointer/rref, 2=reference, 3=const,
                               // 4=volatile, 5=typedef, 6=array
    };
    struct PendingRef {
        int      struct_idx;
        bool     is_base;
        int      item_idx;    // index into bases[] or members[]
        uint64_t type_offset; // absolute .debug_info offset
    };
    std::unordered_map<uint64_t, TypeEntry> type_registry_;
    std::vector<PendingRef> pending_refs_;
};

} // namespace dwarf

#endif  // CORE_API_DWARF_H_
