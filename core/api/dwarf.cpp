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
#include "api/dwarf.h"
#include "base/memory_map.h"
#include <linux/elf.h>
#include <string.h>
#include <stack>
#ifdef __ZSTD__
#include <zstd.h>
#endif

// ELF compressed section support (SHF_COMPRESSED may not be in older kernel headers)
#ifndef SHF_COMPRESSED
#define SHF_COMPRESSED 0x800
#endif
#ifndef ELFCOMPRESS_ZLIB
#define ELFCOMPRESS_ZLIB 1
#endif
#ifndef ELFCOMPRESS_ZSTD
#define ELFCOMPRESS_ZSTD 2
#endif

namespace dwarf {

// DW_FORM constants (not exported in header)
static constexpr uint32_t DW_FORM_addr          = 0x01;
static constexpr uint32_t DW_FORM_block2        = 0x03;
static constexpr uint32_t DW_FORM_block4        = 0x04;
static constexpr uint32_t DW_FORM_data2         = 0x05;
static constexpr uint32_t DW_FORM_data4         = 0x06;
static constexpr uint32_t DW_FORM_data8         = 0x07;
static constexpr uint32_t DW_FORM_string        = 0x08;
static constexpr uint32_t DW_FORM_block         = 0x09;
static constexpr uint32_t DW_FORM_block1        = 0x0a;
static constexpr uint32_t DW_FORM_data1         = 0x0b;
static constexpr uint32_t DW_FORM_flag          = 0x0c;
static constexpr uint32_t DW_FORM_sdata         = 0x0d;
static constexpr uint32_t DW_FORM_strp          = 0x0e;
static constexpr uint32_t DW_FORM_udata         = 0x0f;
static constexpr uint32_t DW_FORM_ref_addr      = 0x10;
static constexpr uint32_t DW_FORM_ref1          = 0x11;
static constexpr uint32_t DW_FORM_ref2          = 0x12;
static constexpr uint32_t DW_FORM_ref4          = 0x13;
static constexpr uint32_t DW_FORM_ref8          = 0x14;
static constexpr uint32_t DW_FORM_ref_udata     = 0x15;
static constexpr uint32_t DW_FORM_indirect      = 0x16;
static constexpr uint32_t DW_FORM_sec_offset    = 0x17;
static constexpr uint32_t DW_FORM_exprloc       = 0x18;
static constexpr uint32_t DW_FORM_flag_present  = 0x19;
static constexpr uint32_t DW_FORM_strx          = 0x1a;
static constexpr uint32_t DW_FORM_addrx         = 0x1b;
static constexpr uint32_t DW_FORM_ref_sup4      = 0x1c;
static constexpr uint32_t DW_FORM_strp_sup      = 0x1d;
static constexpr uint32_t DW_FORM_data16        = 0x1e;
static constexpr uint32_t DW_FORM_line_strp     = 0x1f;
static constexpr uint32_t DW_FORM_ref_sig8      = 0x20;
static constexpr uint32_t DW_FORM_implicit_const = 0x21;
static constexpr uint32_t DW_FORM_loclistx      = 0x22;
static constexpr uint32_t DW_FORM_rnglistx      = 0x23;
static constexpr uint32_t DW_FORM_ref_sup8      = 0x24;
static constexpr uint32_t DW_FORM_strx1         = 0x25;
static constexpr uint32_t DW_FORM_strx2         = 0x26;
static constexpr uint32_t DW_FORM_strx3         = 0x27;
static constexpr uint32_t DW_FORM_strx4         = 0x28;
static constexpr uint32_t DW_FORM_addrx1        = 0x29;
static constexpr uint32_t DW_FORM_addrx2        = 0x2a;
static constexpr uint32_t DW_FORM_addrx3        = 0x2b;
static constexpr uint32_t DW_FORM_addrx4        = 0x2c;

// DW_CHILDREN
static constexpr uint8_t DW_CHILDREN_yes = 1;

// DW_UT (unit type for DWARF 5)
static constexpr uint8_t DW_UT_compile = 0x01;

// ULEB128 / SLEB128
uint64_t DwarfLoader::ReadULEB128(const uint8_t** ptr, const uint8_t* end) {
    uint64_t result = 0;
    int shift = 0;
    while (*ptr < end && shift < 64) {
        uint8_t b = **ptr;
        (*ptr)++;
        result |= (uint64_t)(b & 0x7f) << shift;
        shift += 7;
        if (!(b & 0x80)) break;
    }
    return result;
}

int64_t DwarfLoader::ReadSLEB128(const uint8_t** ptr, const uint8_t* end) {
    int64_t result = 0;
    int shift = 0;
    uint8_t b = 0;
    while (*ptr < end && shift < 64) {
        b = **ptr;
        (*ptr)++;
        result |= (int64_t)(b & 0x7f) << shift;
        shift += 7;
        if (!(b & 0x80)) break;
    }
    if (shift < 64 && (b & 0x40))
        result |= -(int64_t)(1LL << shift);
    return result;
}

// Returns true for CU-relative reference forms.
static bool IsRelRef(uint32_t form) {
    return form == DW_FORM_ref1 || form == DW_FORM_ref2 || form == DW_FORM_ref4 ||
           form == DW_FORM_ref8 || form == DW_FORM_ref_udata;
}

// debug_str lookup
const char* DwarfLoader::GetDebugStr(uint64_t offset) const {
    if (!debug_str_data_ || offset >= debug_str_size_)
        return nullptr;
    // ensure null-terminated within bounds
    const char* s = reinterpret_cast<const char*>(debug_str_data_ + offset);
    const char* end = reinterpret_cast<const char*>(debug_str_data_ + debug_str_size_);
    const char* p = s;
    while (p < end && *p) ++p;
    if (p >= end) return nullptr;
    return s;
}

// DWARF 5: DW_FORM_strx* → index into .debug_str_offsets[str_offsets_base + index*N]
//          → offset into .debug_str
const char* DwarfLoader::GetDebugStrX(uint64_t index, uint64_t str_offsets_base,
                                       bool dwarf64) const {
    if (!debug_str_offsets_data_) return nullptr;
    uint64_t entry_size = dwarf64 ? 8 : 4;
    uint64_t off = str_offsets_base + index * entry_size;
    if (off + entry_size > debug_str_offsets_size_) return nullptr;
    const uint8_t* p = debug_str_offsets_data_ + off;
    uint64_t str_off;
    if (dwarf64) {
        str_off = (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16)
                | ((uint64_t)p[3] << 24) | ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40)
                | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
    } else {
        str_off = (uint32_t)p[0] | ((uint32_t)p[1] << 8)
                | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    }
    return GetDebugStr(str_off);
}

// ParseExprloc
bool DwarfLoader::ParseExprloc(const uint8_t* data, uint64_t size, uint64_t* out_value) {
    if (!data || size == 0) return false;
    const uint8_t* end = data + size;
    if (data[0] == DW_OP_plus_uconst) {
        const uint8_t* p = data + 1;
        if (p >= end) return false;
        *out_value = ReadULEB128(&p, end);
        return true;
    }
    // DW_OP_constu (0x10) + value — also common
    if (data[0] == 0x10) {
        const uint8_t* p = data + 1;
        if (p >= end) return false;
        *out_value = ReadULEB128(&p, end);
        return true;
    }
    return false;
}

// Advances *ptr past the attribute value. Writes to out_value / out_str if
// non-null and the form provides the respective type.
// Returns updated ptr, or nullptr on fatal parse error.
const uint8_t* DwarfLoader::ReadAttrValue(const uint8_t* ptr, const uint8_t* end,
                                           uint32_t form, uint32_t addr_size,
                                           bool dwarf64, uint64_t str_offsets_base,
                                           uint64_t* out_value, std::string* out_str) {
    if (!ptr || ptr >= end) return nullptr;

    auto read_u8  = [&]() -> uint8_t  { uint8_t  v = *ptr; ptr += 1; return v; };
    auto read_u16 = [&]() -> uint16_t {
        uint16_t v = (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
        ptr += 2; return v;
    };
    auto read_u32 = [&]() -> uint32_t {
        uint32_t v = (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8)
                   | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
        ptr += 4; return v;
    };
    auto read_u64 = [&]() -> uint64_t {
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v |= (uint64_t)ptr[i] << (8 * i);
        ptr += 8; return v;
    };
    // offset size depends on DWARF 32/64-bit encoding
    auto read_offset = [&]() -> uint64_t {
        return dwarf64 ? read_u64() : read_u32();
    };

    switch (form) {
    case DW_FORM_addr:
        if (ptr + addr_size > end) return nullptr;
        if (out_value) {
            uint64_t v = 0;
            for (uint32_t i = 0; i < addr_size; ++i) v |= (uint64_t)ptr[i] << (8 * i);
            *out_value = v;
        }
        ptr += addr_size;
        break;

    case DW_FORM_data1:
        if (ptr + 1 > end) return nullptr;
        if (out_value) *out_value = read_u8(); else ptr += 1;
        break;

    case DW_FORM_data2:
        if (ptr + 2 > end) return nullptr;
        if (out_value) *out_value = read_u16(); else ptr += 2;
        break;

    case DW_FORM_data4:
        if (ptr + 4 > end) return nullptr;
        if (out_value) *out_value = read_u32(); else ptr += 4;
        break;

    case DW_FORM_data8:
        if (ptr + 8 > end) return nullptr;
        if (out_value) *out_value = read_u64(); else ptr += 8;
        break;

    case DW_FORM_data16:
        if (ptr + 16 > end) return nullptr;
        ptr += 16;
        break;

    case DW_FORM_udata:
    case DW_FORM_ref_udata: {
        uint64_t v = ReadULEB128(&ptr, end);
        if (out_value) *out_value = v;
        break;
    }

    case DW_FORM_loclistx:
    case DW_FORM_rnglistx:
        ReadULEB128(&ptr, end); // skip index, not needed
        break;

    case DW_FORM_addrx: {
        ReadULEB128(&ptr, end); // skip address index
        break;
    }

    case DW_FORM_strx: {
        uint64_t idx = ReadULEB128(&ptr, end);
        if (out_str) {
            const char* s = GetDebugStrX(idx, str_offsets_base, dwarf64);
            if (s) *out_str = s;
        }
        break;
    }

    case DW_FORM_addrx1:
        if (ptr + 1 > end) return nullptr;
        ptr += 1;
        break;
    case DW_FORM_addrx2:
        if (ptr + 2 > end) return nullptr;
        ptr += 2;
        break;
    case DW_FORM_addrx3:
        if (ptr + 3 > end) return nullptr;
        ptr += 3;
        break;
    case DW_FORM_addrx4:
        if (ptr + 4 > end) return nullptr;
        ptr += 4;
        break;

    case DW_FORM_strx1: {
        if (ptr + 1 > end) return nullptr;
        uint64_t idx = *ptr++;
        if (out_str) {
            const char* s = GetDebugStrX(idx, str_offsets_base, dwarf64);
            if (s) *out_str = s;
        }
        break;
    }
    case DW_FORM_strx2: {
        if (ptr + 2 > end) return nullptr;
        uint64_t idx = (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
        ptr += 2;
        if (out_str) {
            const char* s = GetDebugStrX(idx, str_offsets_base, dwarf64);
            if (s) *out_str = s;
        }
        break;
    }
    case DW_FORM_strx3: {
        if (ptr + 3 > end) return nullptr;
        uint64_t idx = (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8) | ((uint32_t)ptr[2] << 16);
        ptr += 3;
        if (out_str) {
            const char* s = GetDebugStrX(idx, str_offsets_base, dwarf64);
            if (s) *out_str = s;
        }
        break;
    }
    case DW_FORM_strx4: {
        if (ptr + 4 > end) return nullptr;
        uint64_t idx = (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8)
                     | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
        ptr += 4;
        if (out_str) {
            const char* s = GetDebugStrX(idx, str_offsets_base, dwarf64);
            if (s) *out_str = s;
        }
        break;
    }

    case DW_FORM_sdata: {
        int64_t v = ReadSLEB128(&ptr, end);
        if (out_value) *out_value = (uint64_t)v;
        break;
    }

    case DW_FORM_string: {
        const char* s = reinterpret_cast<const char*>(ptr);
        while (ptr < end && *ptr) ++ptr;
        if (ptr >= end) return nullptr;
        if (out_str) *out_str = s;
        ++ptr; // skip null terminator
        break;
    }

    case DW_FORM_strp:
    case DW_FORM_strp_sup:
    case DW_FORM_line_strp: {
        if (ptr + (dwarf64 ? 8 : 4) > end) return nullptr;
        uint64_t off = read_offset();
        if (form == DW_FORM_strp && out_str) {
            const char* s = GetDebugStr(off);
            if (s) *out_str = s;
        }
        break;
    }

    case DW_FORM_block1: {
        if (ptr + 1 > end) return nullptr;
        uint64_t len = read_u8();
        if (ptr + len > end) return nullptr;
        if (out_value) ParseExprloc(ptr, len, out_value);
        ptr += len;
        break;
    }

    case DW_FORM_block2: {
        if (ptr + 2 > end) return nullptr;
        uint64_t len = read_u16();
        if (ptr + len > end) return nullptr;
        if (out_value) ParseExprloc(ptr, len, out_value);
        ptr += len;
        break;
    }

    case DW_FORM_block4: {
        if (ptr + 4 > end) return nullptr;
        uint64_t len = read_u32();
        if (ptr + len > end) return nullptr;
        if (out_value) ParseExprloc(ptr, len, out_value);
        ptr += len;
        break;
    }

    case DW_FORM_block:
    case DW_FORM_exprloc: {
        uint64_t len = ReadULEB128(&ptr, end);
        if (ptr + len > end) return nullptr;
        if (out_value) ParseExprloc(ptr, len, out_value);
        ptr += len;
        break;
    }

    case DW_FORM_flag:
        if (ptr + 1 > end) return nullptr;
        if (out_value) *out_value = read_u8(); else ptr += 1;
        break;

    case DW_FORM_flag_present:
        // zero bytes
        if (out_value) *out_value = 1;
        break;

    case DW_FORM_ref1:
        if (ptr + 1 > end) return nullptr;
        if (out_value) *out_value = read_u8(); else ptr += 1;
        break;

    case DW_FORM_ref2:
        if (ptr + 2 > end) return nullptr;
        if (out_value) *out_value = read_u16(); else ptr += 2;
        break;

    case DW_FORM_ref4:
    case DW_FORM_ref_sup4:
        if (ptr + 4 > end) return nullptr;
        if (out_value) *out_value = read_u32(); else ptr += 4;
        break;

    case DW_FORM_ref8:
    case DW_FORM_ref_sup8:
    case DW_FORM_ref_sig8:
        if (ptr + 8 > end) return nullptr;
        if (out_value) *out_value = read_u64(); else ptr += 8;
        break;

    case DW_FORM_ref_addr:
    case DW_FORM_sec_offset:
        if (ptr + (dwarf64 ? 8 : 4) > end) return nullptr;
        if (out_value) *out_value = read_offset(); else ptr += (dwarf64 ? 8 : 4);
        break;

    case DW_FORM_indirect: {
        uint32_t real_form = (uint32_t)ReadULEB128(&ptr, end);
        return ReadAttrValue(ptr, end, real_form, addr_size, dwarf64,
                             str_offsets_base, out_value, out_str);
    }

    case DW_FORM_implicit_const:
        // value was stored in abbrev entry, nothing in .debug_info
        break;

    default:
        // Unknown form — we cannot advance safely.
        // Return nullptr to signal parse error to the caller.
        LOGW("unknown form 0x%x, stopping CU parse\n", form);
        return nullptr;
    }

    return ptr;
}

const uint8_t* DwarfLoader::ParseAbbrevTable(const uint8_t* ptr, const uint8_t* end,
                                              uint64_t base_offset) {
    AbbrevTable table;

    while (ptr < end) {
        uint64_t code = ReadULEB128(&ptr, end);
        if (code == 0) break; // end of table

        AbbrevEntry entry;
        entry.code = (uint32_t)code;
        entry.tag  = (uint32_t)ReadULEB128(&ptr, end);

        if (ptr >= end) break;
        entry.has_children = (*ptr++ == DW_CHILDREN_yes);

        // Read attribute specs
        while (ptr < end) {
            uint32_t attr_name = (uint32_t)ReadULEB128(&ptr, end);
            uint32_t attr_form = (uint32_t)ReadULEB128(&ptr, end);
            if (attr_name == 0 && attr_form == 0) break;

            AttrSpec spec;
            spec.name = attr_name;
            spec.form = attr_form;
            spec.implicit_const = 0;

            if (attr_form == DW_FORM_implicit_const) {
                // DWARF 5: the value follows as SLEB128 in the abbrev table
                spec.implicit_const = ReadSLEB128(&ptr, end);
            }

            entry.attrs.push_back(spec);
        }

        table[entry.code] = entry;
    }

    abbrev_map_[base_offset] = std::move(table);
    return ptr;
}

void DwarfLoader::ParseAbbrevSection() {
    if (!debug_abbrev_data_) return;

    const uint8_t* ptr = debug_abbrev_data_;
    const uint8_t* end = debug_abbrev_data_ + debug_abbrev_size_;

    while (ptr < end) {
        uint64_t offset = (uint64_t)(ptr - debug_abbrev_data_);
        const uint8_t* next = ParseAbbrevTable(ptr, end, offset);
        if (!next || next <= ptr) break;
        ptr = next;
    }
    LOGD("abbrev_size=%lu  tables=%zu\n",
         (unsigned long)debug_abbrev_size_, abbrev_map_.size());
}

const uint8_t* DwarfLoader::ParseDIETree(const uint8_t* ptr, const uint8_t* cu_end,
                                          uint32_t addr_size, bool dwarf64,
                                          const AbbrevTable& abbrev_table,
                                          const std::string& ns_prefix,
                                          uint64_t str_offsets_base,
                                          uint64_t cu_base_offset) {
    // Use index instead of pointer to avoid dangling refs after structs_ reallocation.
    static constexpr int kNoStruct = -1;

    struct StackFrame {
        std::string ns_prefix;
        int struct_idx; // index into structs_, or kNoStruct
    };

    std::stack<StackFrame> stk;
    std::string cur_ns = ns_prefix;
    int cur_struct_idx = kNoStruct;

    while (ptr < cu_end) {
        uint64_t die_abs_offset = (uint64_t)(ptr - debug_info_data_);
        uint64_t code = ReadULEB128(&ptr, cu_end);

        if (code == 0) {
            // Null DIE: pop scope
            if (stk.empty()) break;
            cur_ns         = stk.top().ns_prefix;
            cur_struct_idx = stk.top().struct_idx;
            stk.pop();
            continue;
        }

        auto it = abbrev_table.find((uint32_t)code);
        if (it == abbrev_table.end()) {
            // Corrupted or unknown abbrev code — stop parsing this CU
            LOGW("unknown abbrev code %lu, stopping CU parse\n", (unsigned long)code);
            return nullptr;
        }

        const AbbrevEntry& entry = it->second;

        // Read all attributes
        std::string die_name;
        uint64_t member_loc  = 0;  bool has_member_loc = false;
        uint64_t byte_sz     = 0;  bool has_byte_sz    = false;
        uint64_t type_ref    = 0;  uint32_t type_ref_form = 0;

        for (const AttrSpec& attr : entry.attrs) {
            uint64_t v = 0;
            std::string s;

            if (attr.form == DW_FORM_implicit_const) {
                v = (uint64_t)attr.implicit_const;
                // no bytes consumed from .debug_info
            } else {
                const uint8_t* next = ReadAttrValue(ptr, cu_end, attr.form,
                                                     addr_size, dwarf64,
                                                     str_offsets_base, &v, &s);
                if (!next) {
                    // parse error; skip to end of CU
                    return nullptr;
                }
                ptr = next;
            }

            switch (attr.name) {
            case DW_AT_name:
                if (!s.empty()) die_name = s;
                break;
            case DW_AT_data_member_location:
                member_loc = v;
                has_member_loc = true;
                break;
            case DW_AT_byte_size:
                byte_sz = v;
                has_byte_sz = true;
                break;
            case DW_AT_type:
                type_ref = v;
                type_ref_form = attr.form;
                break;
            default:
                break;
            }
        }

        // Convert CU-relative type ref to absolute .debug_info offset
        uint64_t abs_type_ref = type_ref_form ?
            (IsRelRef(type_ref_form) ? cu_base_offset + type_ref : type_ref) : 0;

        // Handle DIE by tag
        switch (entry.tag) {
        case DW_TAG_namespace: {
            if (entry.has_children) {
                std::string new_ns = cur_ns.empty() ? die_name
                                                    : (cur_ns + "::" + die_name);
                stk.push({cur_ns, cur_struct_idx});
                cur_ns = new_ns;
                // cur_struct_idx unchanged — namespace doesn't reset struct context
            }
            break;
        }

        case DW_TAG_structure_type:
        case DW_TAG_class_type: {
            if (!die_name.empty()) {
                std::string full_name = cur_ns.empty() ? die_name
                                                       : (cur_ns + "::" + die_name);
                // Register in type registry so members/bases can reference this type
                type_registry_[die_abs_offset] = {full_name, 0, 0};
                // push_back may reallocate; record index before, set after
                structs_.push_back({full_name, (uint32_t)byte_sz, has_byte_sz, {}, {}});
                int new_idx = (int)structs_.size() - 1;
                if (entry.has_children) {
                    stk.push({cur_ns, cur_struct_idx});
                    cur_struct_idx = new_idx;
                }
            } else {
                // anonymous struct/class
                if (entry.has_children) {
                    stk.push({cur_ns, cur_struct_idx});
                    cur_struct_idx = kNoStruct;
                }
            }
            break;
        }

        case DW_TAG_member: {
            if (cur_struct_idx != kNoStruct && !die_name.empty()) {
                structs_[cur_struct_idx].members.push_back(
                    {die_name, "", (uint32_t)member_loc, has_member_loc});
                if (abs_type_ref) {
                    int member_idx = (int)structs_[cur_struct_idx].members.size() - 1;
                    pending_refs_.push_back({cur_struct_idx, false, member_idx, abs_type_ref});
                }
            }
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;
        }

        case DW_TAG_inheritance: {
            if (cur_struct_idx != kNoStruct) {
                structs_[cur_struct_idx].bases.push_back(
                    {"", (uint32_t)member_loc, has_member_loc});
                if (abs_type_ref) {
                    int base_idx = (int)structs_[cur_struct_idx].bases.size() - 1;
                    pending_refs_.push_back({cur_struct_idx, true, base_idx, abs_type_ref});
                }
            }
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;
        }

        case DW_TAG_base_type:
        case DW_TAG_enumeration_type:
        case DW_TAG_union_type: {
            if (!die_name.empty()) {
                std::string full_name = cur_ns.empty() ? die_name
                                                       : (cur_ns + "::" + die_name);
                type_registry_[die_abs_offset] = {full_name, 0, 0};
            }
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;
        }

        case DW_TAG_typedef: {
            if (!die_name.empty()) {
                std::string full_name = cur_ns.empty() ? die_name
                                                       : (cur_ns + "::" + die_name);
                type_registry_[die_abs_offset] = {full_name, abs_type_ref, 5};
            }
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;
        }

        case DW_TAG_pointer_type:
        case DW_TAG_rvalue_reference_type:
            type_registry_[die_abs_offset] = {"", abs_type_ref, 1};
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;

        case DW_TAG_reference_type:
            type_registry_[die_abs_offset] = {"", abs_type_ref, 2};
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;

        case DW_TAG_const_type:
            type_registry_[die_abs_offset] = {"", abs_type_ref, 3};
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;

        case DW_TAG_volatile_type:
            type_registry_[die_abs_offset] = {"", abs_type_ref, 4};
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;

        case DW_TAG_array_type:
            type_registry_[die_abs_offset] = {"", abs_type_ref, 6};
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;

        default: {
            // Other tags (subprogram, …): push to skip children scope
            if (entry.has_children) {
                stk.push({cur_ns, cur_struct_idx});
                cur_struct_idx = kNoStruct;
            }
            break;
        }
        }
    }

    return ptr;
}

const uint8_t* DwarfLoader::ParseCompileUnit(const uint8_t* ptr, const uint8_t* end) {
    if (ptr + 4 > end) return nullptr;

    const uint8_t* cu_start = ptr; // CU-relative refs are relative to this

    // unit_length
    uint32_t unit_length_32 = (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8)
                             | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
    ptr += 4;

    bool dwarf64 = false;
    uint64_t unit_length;

    if (unit_length_32 == 0xffffffff) {
        // 64-bit DWARF
        if (ptr + 8 > end) return nullptr;
        unit_length = 0;
        for (int i = 0; i < 8; ++i) unit_length |= (uint64_t)ptr[i] << (8 * i);
        ptr += 8;
        dwarf64 = true;
    } else {
        unit_length = unit_length_32;
    }

    const uint8_t* cu_end = ptr + unit_length;
    if (cu_end > end) return nullptr;

    if (ptr + 2 > cu_end) return cu_end;
    uint16_t version = (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);
    ptr += 2;

    uint64_t abbrev_offset;
    uint32_t addr_size;

    if (version >= 5) {
        // DWARF 5: unit_type (1 byte) + address_size (1 byte) + abbrev_offset
        if (ptr + 2 > cu_end) return cu_end;
        uint8_t unit_type = *ptr++;
        addr_size = *ptr++;
        if (unit_type != DW_UT_compile) {
            // Skip non-compile units (type units, etc.)
            return cu_end;
        }
        if (dwarf64) {
            if (ptr + 8 > cu_end) return cu_end;
            abbrev_offset = 0;
            for (int i = 0; i < 8; ++i) abbrev_offset |= (uint64_t)ptr[i] << (8 * i);
            ptr += 8;
        } else {
            if (ptr + 4 > cu_end) return cu_end;
            abbrev_offset = (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8)
                          | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
            ptr += 4;
        }
    } else {
        // DWARF 2-4: abbrev_offset + address_size
        if (dwarf64) {
            if (ptr + 8 > cu_end) return cu_end;
            abbrev_offset = 0;
            for (int i = 0; i < 8; ++i) abbrev_offset |= (uint64_t)ptr[i] << (8 * i);
            ptr += 8;
        } else {
            if (ptr + 4 > cu_end) return cu_end;
            abbrev_offset = (uint32_t)ptr[0] | ((uint32_t)ptr[1] << 8)
                          | ((uint32_t)ptr[2] << 16) | ((uint32_t)ptr[3] << 24);
            ptr += 4;
        }
        if (ptr + 1 > cu_end) return cu_end;
        addr_size = *ptr++;
    }

    // Print first 3 CUs only to avoid flooding output on large .so files
    static int cu_diag_count = 0;
    bool diag = (cu_diag_count++ < 3);
    if (diag)
        LOGD("CU#%d version=%u  unit_length=%lu  addr_size=%u"
             "  abbrev_offset=%lu  dwarf64=%d\n",
             cu_diag_count, version, (unsigned long)unit_length, addr_size,
             (unsigned long)abbrev_offset, (int)dwarf64);

    // Locate the abbrev table for this CU
    auto it = abbrev_map_.find(abbrev_offset);
    if (it == abbrev_map_.end()) {
        LOGW("abbrev table not found at offset %lu\n", (unsigned long)abbrev_offset);
        return cu_end;
    }

    // Peek at the compile_unit DIE to read DW_AT_str_offsets_base (DWARF 5).
    // We scan its attributes without advancing ptr, so ParseDIETree still
    // processes the full CU starting from the compile_unit DIE.
    uint64_t str_offsets_base = 0;
    {
        const uint8_t* peek = ptr;
        uint64_t cu_die_code = ReadULEB128(&peek, cu_end);
        auto cu_die_it = it->second.find((uint32_t)cu_die_code);
        if (cu_die_it != it->second.end()) {
            for (const AttrSpec& attr : cu_die_it->second.attrs) {
                uint64_t v = 0;
                if (attr.form == DW_FORM_implicit_const) {
                    v = (uint64_t)attr.implicit_const;
                } else {
                    const uint8_t* next = ReadAttrValue(peek, cu_end, attr.form,
                                                         addr_size, dwarf64, 0,
                                                         (attr.name == DW_AT_str_offsets_base) ? &v : nullptr,
                                                         nullptr);
                    if (!next) break;
                    peek = next;
                }
                if (attr.name == DW_AT_str_offsets_base) {
                    str_offsets_base = v;
                    break;
                }
            }
        }
    }

    if (diag)
        LOGD("str_offsets_base=%lu  abbrev_entries=%zu\n",
             (unsigned long)str_offsets_base, it->second.size());

    uint64_t cu_base_offset = (uint64_t)(cu_start - debug_info_data_);
    size_t structs_before = structs_.size();
    const uint8_t* die_end = ParseDIETree(ptr, cu_end, addr_size, dwarf64,
                                           it->second, "", str_offsets_base, cu_base_offset);
    if (diag)
        LOGD("structs found in this CU: %zu  (die_end=%s)\n",
             structs_.size() - structs_before, die_end ? "ok" : "parse_error");
    return cu_end;
}

std::string DwarfLoader::ResolveTypeName(uint64_t offset, int depth) const {
    if (depth > 8 || offset == 0) return "";
    auto it = type_registry_.find(offset);
    if (it == type_registry_.end()) return "";
    const TypeEntry& e = it->second;
    switch (e.qualifier) {
    case 0: return e.name;                                                       // named type
    case 1: return ResolveTypeName(e.type_ref, depth + 1) + "*";                // pointer/rref
    case 2: return ResolveTypeName(e.type_ref, depth + 1) + "&";                // reference
    case 3: { auto s = ResolveTypeName(e.type_ref, depth + 1);                  // const
              return s.empty() ? "const" : "const " + s; }
    case 4: { auto s = ResolveTypeName(e.type_ref, depth + 1);                  // volatile
              return s.empty() ? "volatile" : "volatile " + s; }
    case 5: return !e.name.empty() ? e.name : ResolveTypeName(e.type_ref, depth + 1); // typedef
    case 6: return ResolveTypeName(e.type_ref, depth + 1) + "[]";               // array
    default: return e.name;
    }
}

void DwarfLoader::ResolveTypes() {
    for (const auto& p : pending_refs_) {
        if (p.type_offset == 0) continue;
        std::string name = ResolveTypeName(p.type_offset, 0);
        if (name.empty()) continue;
        if (p.is_base)
            structs_[p.struct_idx].bases[p.item_idx].type_name = name;
        else
            structs_[p.struct_idx].members[p.item_idx].type_name = name;
    }
}

void DwarfLoader::ParseInfoSection() {
    if (!debug_info_data_) return;

    const uint8_t* ptr = debug_info_data_;
    const uint8_t* end = debug_info_data_ + debug_info_size_;

    int cu_count = 0;
    while (ptr < end) {
        const uint8_t* next = ParseCompileUnit(ptr, end);
        if (!next || next <= ptr) break;
        ptr = next;
        ++cu_count;
    }
    LOGD("total CUs processed=%d  total structs=%zu\n",
         cu_count, structs_.size());
    ResolveTypes();
}

// Handles ELF SHF_COMPRESSED sections (Elf_Chdr prefix + compressed payload).
// Stores decompressed data in decompressed_sections_ and points out_data/out_size
// at the result. Returns false if unsupported or decompression fails.
bool DwarfLoader::DecompressSection(const uint8_t* raw, uint64_t raw_size,
                                    const uint8_t** out_data, uint64_t* out_size) {
    // Elf64_Chdr: ch_type(4) + ch_reserved(4) + ch_size(8) + ch_addralign(8) = 24 bytes
    // Elf32_Chdr: ch_type(4) + ch_size(4) + ch_addralign(4)                  = 12 bytes
    uint64_t chdr_size = (elf_bits_ == 64) ? 24 : 12;
    if (raw_size < chdr_size) return false;

    uint32_t ch_type = (uint32_t)raw[0] | ((uint32_t)raw[1] << 8)
                     | ((uint32_t)raw[2] << 16) | ((uint32_t)raw[3] << 24);

    uint64_t ch_size;
    if (elf_bits_ == 64) {
        ch_size = (uint64_t)raw[8]  | ((uint64_t)raw[9]  << 8)
                | ((uint64_t)raw[10] << 16) | ((uint64_t)raw[11] << 24)
                | ((uint64_t)raw[12] << 32) | ((uint64_t)raw[13] << 40)
                | ((uint64_t)raw[14] << 48) | ((uint64_t)raw[15] << 56);
    } else {
        ch_size = (uint32_t)raw[4] | ((uint32_t)raw[5] << 8)
                | ((uint32_t)raw[6] << 16) | ((uint32_t)raw[7] << 24);
    }

    const uint8_t* payload = raw + chdr_size;
    uint64_t payload_size  = raw_size - chdr_size;

    if (ch_type == ELFCOMPRESS_ZSTD) {
#ifdef __ZSTD__
        decompressed_sections_.emplace_back(ch_size);
        auto& buf = decompressed_sections_.back();
        size_t result = ZSTD_decompress(buf.data(), ch_size, payload, payload_size);
        if (ZSTD_isError(result)) {
            decompressed_sections_.pop_back();
            LOGW("zstd decompress failed: %s\n", ZSTD_getErrorName(result));
            return false;
        }
        buf.resize(result);
        *out_data = buf.data();
        *out_size = result;
        return true;
#else
        LOGW("section is zstd-compressed but zstd support not compiled in\n");
        return false;
#endif
    } else if (ch_type == ELFCOMPRESS_ZLIB) {
        LOGW("zlib-compressed sections not supported\n");
        return false;
    }

    LOGW("unknown compression type %u\n", ch_type);
    return false;
}

bool DwarfLoader::LocateSections() {
    if (!map_) return false;

    const uint8_t* data = reinterpret_cast<const uint8_t*>(map_->data());
    uint64_t size = map_->size();

    if (size < 5 || memcmp(data, "\x7f""ELF", 4) != 0) {
        LOGW("not a valid ELF file\n");
        return false;
    }

    elf_bits_ = (data[4] == 1) ? 32 : 64;

    auto locate = [&](const char* shstr, const void* shdr_base, int sh_num,
                      uint64_t sh_entsize) {
        for (int i = 0; i < sh_num; ++i) {
            const uint8_t* shdr_ptr = reinterpret_cast<const uint8_t*>(shdr_base) + i * sh_entsize;
            uint32_t sh_type;
            uint64_t sh_offset, sh_size, sh_name_idx, sh_flags;

            if (elf_bits_ == 64) {
                const Elf64_Shdr* s = reinterpret_cast<const Elf64_Shdr*>(shdr_ptr);
                sh_type     = s->sh_type;
                sh_offset   = s->sh_offset;
                sh_size     = s->sh_size;
                sh_name_idx = s->sh_name;
                sh_flags    = s->sh_flags;
            } else {
                const Elf32_Shdr* s = reinterpret_cast<const Elf32_Shdr*>(shdr_ptr);
                sh_type     = s->sh_type;
                sh_offset   = s->sh_offset;
                sh_size     = s->sh_size;
                sh_name_idx = s->sh_name;
                sh_flags    = s->sh_flags;
            }

            if (sh_type == SHT_NOBITS) continue;
            if (sh_offset + sh_size > size) continue;

            // bounds-check name access
            const char* name_ptr = shstr + sh_name_idx;
            const uint8_t* name_addr = reinterpret_cast<const uint8_t*>(name_ptr);
            if (name_addr < data || name_addr >= data + size) continue;

            const char* name = name_ptr;
            const uint8_t** sec_data = nullptr;
            uint64_t*        sec_size = nullptr;

            if      (!strcmp(name, ".debug_abbrev"))      { sec_data = &debug_abbrev_data_;      sec_size = &debug_abbrev_size_; }
            else if (!strcmp(name, ".debug_info"))        { sec_data = &debug_info_data_;        sec_size = &debug_info_size_; }
            else if (!strcmp(name, ".debug_str"))         { sec_data = &debug_str_data_;         sec_size = &debug_str_size_; }
            else if (!strcmp(name, ".debug_str_offsets")) { sec_data = &debug_str_offsets_data_; sec_size = &debug_str_offsets_size_; }

            if (!sec_data) continue;

            if (sh_flags & SHF_COMPRESSED) {
                DecompressSection(data + sh_offset, sh_size, sec_data, sec_size);
            } else {
                *sec_data = data + sh_offset;
                *sec_size = sh_size;
            }
        }
    };

    if (elf_bits_ == 64) {
        if (size < sizeof(Elf64_Ehdr)) return false;
        const Elf64_Ehdr* ehdr = reinterpret_cast<const Elf64_Ehdr*>(data);
        if (!ehdr->e_shoff || ehdr->e_shoff + (uint64_t)ehdr->e_shnum * sizeof(Elf64_Shdr) > size)
            return false;
        if (ehdr->e_shstrndx >= ehdr->e_shnum) return false;
        const Elf64_Shdr* shdr = reinterpret_cast<const Elf64_Shdr*>(data + ehdr->e_shoff);
        if (shdr[ehdr->e_shstrndx].sh_offset >= size) return false;
        const char* shstr = reinterpret_cast<const char*>(data + shdr[ehdr->e_shstrndx].sh_offset);
        locate(shstr, shdr, ehdr->e_shnum, sizeof(Elf64_Shdr));
    } else {
        if (size < sizeof(Elf32_Ehdr)) return false;
        const Elf32_Ehdr* ehdr = reinterpret_cast<const Elf32_Ehdr*>(data);
        if (!ehdr->e_shoff || ehdr->e_shoff + (uint64_t)ehdr->e_shnum * sizeof(Elf32_Shdr) > size)
            return false;
        if (ehdr->e_shstrndx >= ehdr->e_shnum) return false;
        const Elf32_Shdr* shdr = reinterpret_cast<const Elf32_Shdr*>(data + ehdr->e_shoff);
        if (shdr[ehdr->e_shstrndx].sh_offset >= size) return false;
        const char* shstr = reinterpret_cast<const char*>(data + shdr[ehdr->e_shstrndx].sh_offset);
        locate(shstr, shdr, ehdr->e_shnum, sizeof(Elf32_Shdr));
    }

    if (!debug_abbrev_data_ || !debug_info_data_) {
        LOGW(".debug_abbrev/.debug_info sections not found in ELF\n");
        return false;
    }
    LOGD("elf_bits=%d  .debug_abbrev=%lu  .debug_info=%lu"
         "  .debug_str=%lu  .debug_str_offsets=%lu\n",
         elf_bits_,
         (unsigned long)debug_abbrev_size_, (unsigned long)debug_info_size_,
         (unsigned long)debug_str_size_,    (unsigned long)debug_str_offsets_size_);
    return true;
}

bool DwarfLoader::Init(const char* elf_path) {
    map_.reset(MemoryMap::MmapFile(elf_path));
    if (!map_) {
        LOGW("cannot open %s\n", elf_path);
        return false;
    }
    if (!LocateSections()) return false;
    ParseAbbrevSection();
    ParseInfoSection();
    if (structs_.empty()) {
        LOGW("no struct/class types extracted from %s\n"
             "make sure the file is an unstripped debug build\n", elf_path);
        return false;
    }
    return true;
}

std::unique_ptr<DwarfLoader> DwarfLoader::Load(const char* elf_path) {
    auto loader = std::unique_ptr<DwarfLoader>(new DwarfLoader());
    if (!loader->Init(elf_path)) return nullptr;
    return loader;
}

void DwarfLoader::ForEachStruct(std::function<bool (const StructInfo&)> cb) const {
    for (const StructInfo& si : structs_)
        if (cb(si)) break;
}

} // namespace dwarf
