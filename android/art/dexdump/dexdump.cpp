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

#include "dalvik_vm_bytecode.h"
#include "dexdump/dexdump.h"
#include "base/utils.h"
#include "dex/descriptors_names.h"
#include <stdio.h>

namespace art {

uint32_t Dexdump::GetDexInstSize(api::MemoryRef& ref) {
    uint8_t op = GetDexOp(ref);
    if (!IsVaildDexOp(op))
        return 0x2;

    switch (op) {
        case DEXOP::NOP: return !ref.value16Of() ? 0x2 : 0xA;
        case DEXOP::MOVE: return 0x2;
        case DEXOP::MOVE_FROM16: return 0x4;
        case DEXOP::MOVE_16: return 0x6;
        case DEXOP::MOVE_WIDE: return 0x2;
        case DEXOP::MOVE_WIDE_FROM16: return 0x4;
        case DEXOP::MOVE_WIDE_16: return 0x6;
        case DEXOP::MOVE_OBJECT: return 0x2;
        case DEXOP::MOVE_OBJECT_FROM16: return 0x4;
        case DEXOP::MOVE_OBJECT_16: return 0x6;

        case DEXOP::MOVE_RESULT: return 0x2;
        case DEXOP::MOVE_RESULT_WIDE: return 0x2;
        case DEXOP::MOVE_RESULT_OBJECT: return 0x2;
        case DEXOP::MOVE_EXCEPTION: return 0x2;

        case DEXOP::RETURN_VOID: return 0x2;
        case DEXOP::RETURN: return 0x2;
        case DEXOP::RETURN_WIDE: return 0x2;
        case DEXOP::RETURN_OBJECT: return 0x2;

        case DEXOP::CONST_4: return 0x2;
        case DEXOP::CONST_16: return 0x4;
        case DEXOP::CONST: return 0x6;
        case DEXOP::CONST_HIGH16: return 0x4;
        case DEXOP::CONST_WIDE_16: return 0x4;
        case DEXOP::CONST_WIDE_32: return 0x6;
        case DEXOP::CONST_WIDE: return 0xA;
        case DEXOP::CONST_WIDE_HIGH16: return 0x4;
        case DEXOP::CONST_STRING: return 0x4;
        case DEXOP::CONST_STRING_JUMBO: return 0x6;
        case DEXOP::CONST_CLASS: return 0x4;

        case DEXOP::MONITOR_ENTER: return 0x2;
        case DEXOP::MONITOR_EXIT: return 0x2;

        case DEXOP::CHECK_CAST: return 0x4;
        case DEXOP::INSTANCE_OF: return 0x4;
        case DEXOP::ARRAY_LENGTH: return 0x2;
        case DEXOP::NEW_INSTANCE: return 0x4;
        case DEXOP::NEW_ARRAY: return 0x4;
        case DEXOP::FILLED_NEW_ARRAY: return 0x6;
        case DEXOP::FILLED_NEW_ARRAY_RANGE: return 0x6;
        case DEXOP::FILL_ARRAY_DATA: return 0x6;

        case DEXOP::THROW: return 0x2;
        case DEXOP::GOTO: return 0x2;
        case DEXOP::GOTO_16: return 0x4;
        case DEXOP::GOTO_32: return 0x6;

        case DEXOP::PACKED_SWITCH: return 0x6;
        case DEXOP::SPARSE_SWITCH: return 0x6;

        case DEXOP::CMPL_FLOAT:
        case DEXOP::CMPG_FLOAT:
        case DEXOP::CMPL_DOUBLE:
        case DEXOP::CMPG_DOUBLE:
        case DEXOP::CMP_LONG: return 0x4;

        case DEXOP::IF_EQ:
        case DEXOP::IF_NE:
        case DEXOP::IF_LT:
        case DEXOP::IF_GE:
        case DEXOP::IF_GT:
        case DEXOP::IF_LE: return 0x4;

        case DEXOP::IF_EQZ:
        case DEXOP::IF_NEZ:
        case DEXOP::IF_LTZ:
        case DEXOP::IF_GEZ:
        case DEXOP::IF_GTZ:
        case DEXOP::IF_LEZ: return 0x4;

        case DEXOP::AGET:
        case DEXOP::AGET_WIDE:
        case DEXOP::AGET_OBJECT:
        case DEXOP::AGET_BOOLEAN:
        case DEXOP::AGET_BYTE:
        case DEXOP::AGET_CHAR:
        case DEXOP::AGET_SHORT:
        case DEXOP::APUT:
        case DEXOP::APUT_WIDE:
        case DEXOP::APUT_OBJECT:
        case DEXOP::APUT_BOOLEAN:
        case DEXOP::APUT_BYTE:
        case DEXOP::APUT_CHAR:
        case DEXOP::APUT_SHORT: return 0x4;

        case DEXOP::IGET:
        case DEXOP::IGET_WIDE:
        case DEXOP::IGET_OBJECT:
        case DEXOP::IGET_BOOLEAN:
        case DEXOP::IGET_BYTE:
        case DEXOP::IGET_CHAR:
        case DEXOP::IGET_SHORT:
        case DEXOP::IPUT:
        case DEXOP::IPUT_WIDE:
        case DEXOP::IPUT_OBJECT:
        case DEXOP::IPUT_BOOLEAN:
        case DEXOP::IPUT_BYTE:
        case DEXOP::IPUT_CHAR:
        case DEXOP::IPUT_SHORT: return 0x4;

        case DEXOP::SGET:
        case DEXOP::SGET_WIDE:
        case DEXOP::SGET_OBJECT:
        case DEXOP::SGET_BOOLEAN:
        case DEXOP::SGET_BYTE:
        case DEXOP::SGET_CHAR:
        case DEXOP::SGET_SHORT:
        case DEXOP::SPUT:
        case DEXOP::SPUT_WIDE:
        case DEXOP::SPUT_OBJECT:
        case DEXOP::SPUT_BOOLEAN:
        case DEXOP::SPUT_BYTE:
        case DEXOP::SPUT_CHAR:
        case DEXOP::SPUT_SHORT: return 0x4;

        case DEXOP::INVOKE_VIRTUAL:
        case DEXOP::INVOKE_SUPER:
        case DEXOP::INVOKE_DIRECT:
        case DEXOP::INVOKE_STATIC:
        case DEXOP::INVOKE_INTERFACE: return 0x6;

        case DEXOP::INVOKE_VIRTUAL_RANGE:
        case DEXOP::INVOKE_SUPER_RANGE:
        case DEXOP::INVOKE_DIRECT_RANGE:
        case DEXOP::INVOKE_STATIC_RANGE:
        case DEXOP::INVOKE_INTERFACE_RANGE: return 0x6;

        case DEXOP::NEG_INT:
        case DEXOP::NOT_INT:
        case DEXOP::NEG_LONG:
        case DEXOP::NOT_LONG:
        case DEXOP::NEG_FLOAT:
        case DEXOP::NEG_DOUBLE:
        case DEXOP::INT_TO_LONG:
        case DEXOP::INT_TO_FLOAT:
        case DEXOP::INT_TO_DOUBLE:
        case DEXOP::LONG_TO_INT:
        case DEXOP::LONG_TO_FLOAT:
        case DEXOP::LONG_TO_DOUBLE:
        case DEXOP::FLOAT_TO_INT:
        case DEXOP::FLOAT_TO_LONG:
        case DEXOP::FLOAT_TO_DOUBLE:
        case DEXOP::DOUBLE_TO_INT:
        case DEXOP::DOUBLE_TO_LONG:
        case DEXOP::DOUBLE_TO_FLOAT:
        case DEXOP::INT_TO_BYTE:
        case DEXOP::INT_TO_CHAR:
        case DEXOP::INT_TO_SHORT: return 0x2;

        case DEXOP::ADD_INT:
        case DEXOP::SUB_INT:
        case DEXOP::MUL_INT:
        case DEXOP::DIV_INT:
        case DEXOP::REM_INT:
        case DEXOP::AND_INT:
        case DEXOP::OR_INT:
        case DEXOP::XOR_INT:
        case DEXOP::SHL_INT:
        case DEXOP::SHR_INT:
        case DEXOP::USHR_INT:
        case DEXOP::ADD_LONG:
        case DEXOP::SUB_LONG:
        case DEXOP::MUL_LONG:
        case DEXOP::DIV_LONG:
        case DEXOP::REM_LONG:
        case DEXOP::AND_LONG:
        case DEXOP::OR_LONG:
        case DEXOP::XOR_LONG:
        case DEXOP::SHL_LONG:
        case DEXOP::SHR_LONG:
        case DEXOP::USHR_LONG:
        case DEXOP::ADD_FLOAT:
        case DEXOP::SUB_FLOAT:
        case DEXOP::MUL_FLOAT:
        case DEXOP::DIV_FLOAT:
        case DEXOP::REM_FLOAT:
        case DEXOP::ADD_DOUBLE:
        case DEXOP::SUB_DOUBLE:
        case DEXOP::MUL_DOUBLE:
        case DEXOP::DIV_DOUBLE:
        case DEXOP::REM_DOUBLE: return 0x4;

        case DEXOP::ADD_INT_2ADDR:
        case DEXOP::SUB_INT_2ADDR:
        case DEXOP::MUL_INT_2ADDR:
        case DEXOP::DIV_INT_2ADDR:
        case DEXOP::REM_INT_2ADDR:
        case DEXOP::AND_INT_2ADDR:
        case DEXOP::OR_INT_2ADDR:
        case DEXOP::XOR_INT_2ADDR:
        case DEXOP::SHL_INT_2ADDR:
        case DEXOP::SHR_INT_2ADDR:
        case DEXOP::USHR_INT_2ADDR:
        case DEXOP::ADD_LONG_2ADDR:
        case DEXOP::SUB_LONG_2ADDR:
        case DEXOP::MUL_LONG_2ADDR:
        case DEXOP::DIV_LONG_2ADDR:
        case DEXOP::REM_LONG_2ADDR:
        case DEXOP::AND_LONG_2ADDR:
        case DEXOP::OR_LONG_2ADDR:
        case DEXOP::XOR_LONG_2ADDR:
        case DEXOP::SHL_LONG_2ADDR:
        case DEXOP::SHR_LONG_2ADDR:
        case DEXOP::USHR_LONG_2ADDR:
        case DEXOP::ADD_FLOAT_2ADDR:
        case DEXOP::SUB_FLOAT_2ADDR:
        case DEXOP::MUL_FLOAT_2ADDR:
        case DEXOP::DIV_FLOAT_2ADDR:
        case DEXOP::REM_FLOAT_2ADDR:
        case DEXOP::ADD_DOUBLE_2ADDR:
        case DEXOP::SUB_DOUBLE_2ADDR:
        case DEXOP::MUL_DOUBLE_2ADDR:
        case DEXOP::DIV_DOUBLE_2ADDR:
        case DEXOP::REM_DOUBLE_2ADDR: return 0x2;

        case DEXOP::ADD_INT_LIT16:
        case DEXOP::RSUB_INT_LIT16:
        case DEXOP::MUL_INT_LIT16:
        case DEXOP::DIV_INT_LIT16:
        case DEXOP::REM_INT_LIT16:
        case DEXOP::AND_INT_LIT16:
        case DEXOP::OR_INT_LIT16:
        case DEXOP::XOR_INT_LIT16: return 0x4;

        case DEXOP::ADD_INT_LIT8:
        case DEXOP::RSUB_INT_LIT8:
        case DEXOP::MUL_INT_LIT8:
        case DEXOP::DIV_INT_LIT8:
        case DEXOP::REM_INT_LIT8:
        case DEXOP::AND_INT_LIT8:
        case DEXOP::OR_INT_LIT8:
        case DEXOP::XOR_INT_LIT8:
        case DEXOP::SHL_INT_LIT8:
        case DEXOP::SHR_INT_LIT8:
        case DEXOP::USHR_INT_LIT8: return 0x4;

        case DEXOP::INVOKE_POLYMORPHIC: return 0x8;
        case DEXOP::INVOKE_POLYMORPHIC_RANGE: return 0x8;
        case DEXOP::INVOKE_CUSTOM: return 0x6;
        case DEXOP::INVOKE_CUSTOM_RANGE: return 0x6;
        case DEXOP::CONST_METHOD_HANDLE: return 0x4;
        case DEXOP::CONST_METHOD_TYPE: return 0x4;
    }
    return 0x2;
}

void Dexdump::AppendCodecNoArgs(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    snprintf(codec, sizeof(codec), "%04x                     ", ref.value16Of());
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
}

void Dexdump::AppendCodecArgsvAvB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    snprintf(codec, sizeof(codec), "%04x                     ", code0);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t reg = (code0 & 0xFF00) >> 8;
    uint8_t va = reg & 0x0F;
    uint8_t vb = (reg & 0xF0) >> 4;
    sb.append("v");
    sb.append(std::to_string(va));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vb));
}

void Dexdump::AppendCodecArgsvAAvBBBB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    uint16_t vbbbb = code1;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vbbbb));
}

void Dexdump::AppendCodecArgsvAAAAvBBBB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint16_t vaaaa = code1;
    uint16_t vbbbb = code2;
    sb.append("v");
    sb.append(std::to_string(vaaaa));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vbbbb));
}

void Dexdump::AppendCodecArgsvAA(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    snprintf(codec, sizeof(codec), "%04x                     ", code0);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
}

void Dexdump::AppendCodecArgsvAB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    snprintf(codec, sizeof(codec), "%04x                     ", code0);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t va = (code0 & 0x0F00) >> 8;
    sb.append("v");
    sb.append(std::to_string(va));
    sb.append(", ");
    uint8_t b = (code0 & 0xF000) >> 12;
    if (b & 0x8) {
        sb.append("#-");
        sb.append(std::to_string((~((b & 0x7) - 0x1)) & 0x7));
    } else {
        sb.append("#+");
        sb.append(std::to_string(b & 0x7));
    }
}

void Dexdump::AppendCodecArgsvAABBBB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint16_t bbbb = code1;
    if (bbbb & 0x8000) {
        sb.append("#-");
        sb.append(std::to_string(~((bbbb & 0x7FFF) - 0x1) & 0x7FFF));
    } else {
        sb.append("#+");
        sb.append(std::to_string(bbbb & 0x7FFF));
    }
}

void Dexdump::AppendCodecArgsvAABBBBBBBB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint32_t bbbbbbbb = ((uint32_t)code2 & 0xFFFF) << 16 | code1;
    if (bbbbbbbb & 0x80000000) {
        sb.append("#-");
        sb.append(std::to_string(~((bbbbbbbb & 0x7FFFFFFF) - 0x1) & 0x7FFFFFFF));
    } else {
        sb.append("#+");
        sb.append(std::to_string(bbbbbbbb & 0x7FFFFFFF));
    }
}

void Dexdump::AppendCodecArgsvAABBBB0000(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    uint32_t bbbb = code1;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    if (bbbb & 0x8000) {
        uint32_t bbbb0000 = (~((bbbb & 0x7FFF) - 0x1) & 0x7FFF) << 16;
        sb.append("#-");
        sb.append(std::to_string(bbbb0000));
    } else {
        uint32_t bbbb0000 = (bbbb & 0x7FFF) << 16;
        sb.append("#+");
        sb.append(std::to_string(bbbb0000));
    }
}

void Dexdump::AppendCodecArgsvAABBBBBBBBBBBBBBBB(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    uint16_t code3 = ref.value16Of(6);
    uint16_t code4 = ref.value16Of(8);
    snprintf(codec, sizeof(codec), "%04x %04x %04x %04x %04x ", code0, code1, code2, code3, code4);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint64_t bbbbbbbbbbbbbbbb =  (((uint64_t)code4 & 0xFFFF) << 48)
                               | (((uint64_t)code3 & 0xFFFF) << 32)
                               | (((uint64_t)code2 & 0xFFFF) << 16)
                               | (((uint64_t)code1 & 0xFFFF));
    if (bbbbbbbbbbbbbbbb & 0x8000000000000000ULL) {
        sb.append("#-");
        sb.append(std::to_string(~((bbbbbbbbbbbbbbbb & 0x7FFFFFFFFFFFFFFFULL) - 0x1) & 0x7FFFFFFFFFFFFFFFULL));
    } else {
        sb.append("#+");
        sb.append(std::to_string(bbbbbbbbbbbbbbbb & 0x7FFFFFFFFFFFFFFFULL));
    }
}

void Dexdump::AppendCodecArgsvAABBBB000000000000(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    uint64_t bbbb = code1;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    if (bbbb & 0x8000) {
        uint64_t bbbb000000000000 = (~((bbbb & 0x7FFF) - 0x1) & 0x7FFF) << 48;
        sb.append("#-");
        sb.append(std::to_string(bbbb000000000000));
    } else {
        uint64_t bbbb000000000000 = (bbbb & 0x7FFF) << 48;
        sb.append("#+");
        sb.append(std::to_string(bbbb000000000000));
    }
}

void Dexdump::AppendCodecArgsvAAStringBBBB(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    dex::StringIndex sidx(code1);
    sb.append("\"");
    sb.append(dex_file.StringDataByIdx(sidx));
    sb.append("\"");
    sb.append(" // string@");
    sb.append(std::to_string(sidx.Index()));
}

void Dexdump::AppendCodecArgsvAAStringBBBBBBBB(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    dex::StringIndex sidx(((uint32_t)code2 & 0xFFFF) << 16 | code1);
    sb.append("\"");
    sb.append(dex_file.StringDataByIdx(sidx));
    sb.append("\"");
    sb.append(" // string@");
    sb.append(std::to_string(sidx.Index()));
}

void Dexdump::AppendCodecArgsvAATypeBBBB(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    dex::TypeIndex type(code1);
    dex::TypeId tid = dex_file.GetTypeId(type);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetTypeDescriptor(tid), &result);
    sb.append(result);
    sb.append(" // type@");
    sb.append(std::to_string(type.Index()));
}

void Dexdump::AppendCodecArgsvAvBTypeCCCC(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t reg = (code0 & 0xFF00) >> 8;
    int8_t va = reg & 0x0F;
    int8_t vb = (reg & 0xF0) >> 4;
    sb.append("v");
    sb.append(std::to_string(va));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vb));
    sb.append(", ");
    dex::TypeIndex type(code1);
    dex::TypeId tid = dex_file.GetTypeId(type);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetTypeDescriptor(tid), &result);
    sb.append(result);
    sb.append(" // type@");
    sb.append(std::to_string(type.Index()));
    sb.append("]");
}

void Dexdump::AppendCodecArgsAA(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    snprintf(codec, sizeof(codec), "%04x                     ", code0);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    int aa = (code0 & 0xFF00) >> 8;
    if (aa & 0x80) {
        sb.append("#-");
        sb.append(std::to_string(~((aa & 0x7F) - 0x1) & 0x7F));
    } else {
        sb.append("#+");
        sb.append(std::to_string(aa & 0x7F));
    }
}

void Dexdump::AppendCodecArgsAAAA(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint16_t aaaa = code1;
    if (aaaa & 0x8000) {
        sb.append("#-");
        sb.append(std::to_string(~((aaaa & 0x7FFF) - 0x1) & 0x7FFF));
    } else {
        sb.append("#+");
        sb.append(std::to_string(aaaa & 0x7FFF));
    }
}

void Dexdump::AppendCodecArgsAAAAAAAA(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint32_t aaaaaaaa = ((uint32_t)code2 & 0xFFFF) << 16 | code1;
    if (aaaaaaaa & 0x80000000) {
        sb.append("#-");
        sb.append(std::to_string(~((aaaaaaaa & 0x7FFFFFFF) - 0x1) & 0x7FFFFFFF));
    } else {
        sb.append("#+");
        sb.append(std::to_string(aaaaaaaa & 0x7FFFFFFF));
    }
}

void Dexdump::AppendCodecArgsvAAvBBvCC(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint8_t vbb = code1 & 0x00FF;
    sb.append("v");
    sb.append(std::to_string(vbb));
    sb.append(", ");
    uint8_t vcc = (code1 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vcc));
}

void Dexdump::AppendCodecArgsvAvBCCCC(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t reg = (code0 & 0xFF00) >> 8;
    uint8_t va = reg & 0x0F;
    uint8_t vb = (reg & 0xF0) >> 4;
    sb.append("v");
    sb.append(std::to_string(va));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vb));
    sb.append(", ");
    uint16_t cccc = code1;
    if (cccc & 0x8000) {
        sb.append("#-");
        sb.append(std::to_string(~((cccc & 0x7FFF) - 0x1) & 0x7FFF));
    } else {
        sb.append("#+");
        sb.append(std::to_string(cccc & 0x7FFF));
    }
}

void Dexdump::AppendCodecArgsvAAvBBCC(const char* op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(op);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint8_t vbb = code1 & 0x00FF;
    sb.append("v");
    sb.append(std::to_string(vbb));
    sb.append(", ");
    uint8_t cc = (code1 & 0xFF00) >> 8;
    if (cc & 0x80) {
        sb.append("#-");
        sb.append(std::to_string(~((cc & 0x7F) - 0x1) & 0x7F));
    } else {
        sb.append("#+");
        sb.append(std::to_string(cc & 0x7F));
    }
}

void Dexdump::AppendCodecNop(api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    if (!code0) {
        snprintf(codec, sizeof(codec), "%04x                     ", code0);
        sb.append(Logger::LightYellow());
        sb.append(codec);
        sb.append(Logger::End());
        sb.append("| ");
        sb.append(Logger::LightGreen());
    } else {
        uint16_t code1 = ref.value16Of(2);
        uint16_t code2 = ref.value16Of(4);
        uint16_t code3 = ref.value16Of(6);
        uint16_t code4 = ref.value16Of(8);
        snprintf(codec, sizeof(codec), "%04x %04x %04x %04x %04x ", code0, code1, code2, code3, code4);
        sb.append(Logger::LightYellow());
        sb.append(codec);
        sb.append(Logger::End());
        sb.append("| ");
        sb.append(Logger::LightGreen());
    }
    sb.append("nop");
}

void Dexdump::AppendCodecMove(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvB("move", ref, sb);
}

void Dexdump::AppendCodecMoveFrom16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBBB("move/from16", ref, sb);
}

void Dexdump::AppendCodecMove16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAAAvBBBB("move/16", ref, sb);
}

void Dexdump::AppendCodecMoveWide(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvB("move-wide", ref, sb);
}

void Dexdump::AppendCodecMoveWideFrom16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBBB("move-wide/from16", ref, sb);
}

void Dexdump::AppendCodecMoveWide16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAAAvBBBB("move-wide/16", ref, sb);
}

void Dexdump::AppendCodecMoveObject(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvB("move-object", ref, sb);
}

void Dexdump::AppendCodecMoveObjectFrom16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBBB("move-object/from16", ref, sb);
}

void Dexdump::AppendCodecMoveObject16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAAAvBBBB("move-object/16", ref, sb);
}

void Dexdump::AppendCodecMoveResult(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("move-result", ref, sb);
}

void Dexdump::AppendCodecMoveResultWide(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("move-result-wide", ref, sb);
}

void Dexdump::AppendCodecMoveResultObject(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("move-result-object", ref, sb);
}

void Dexdump::AppendCodecMoveException(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("move-exception", ref, sb);
}

void Dexdump::AppendCodecReturnVoid(api::MemoryRef& ref, std::string& sb) {
    AppendCodecNoArgs("return-void", ref, sb);
}

void Dexdump::AppendCodecReturn(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("return", ref, sb);
}

void Dexdump::AppendCodecReturnWide(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("return-wide", ref, sb);
}

void Dexdump::AppendCodecReturnObject(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("return-object", ref, sb);
}

void Dexdump::AppendCodecConst4(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAB("const/4", ref, sb);
}

void Dexdump::AppendCodecConst16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBB("const/16", ref, sb);
}

void Dexdump::AppendCodecConst(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBBBBBB("const", ref, sb);
}

void Dexdump::AppendCodecConstHigh16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBB0000("const/high16", ref, sb);
}

void Dexdump::AppendCodecConstWide16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBB("const-wide/16", ref, sb);
}

void Dexdump::AppendCodecConstWide32(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBBBBBB("const-wide/32", ref, sb);
}

void Dexdump::AppendCodecConstWide(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBBBBBBBBBBBBBB("const-wide", ref, sb);
}

void Dexdump::AppendCodecConstWideHigh16(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBB000000000000("const-wide/high16", ref, sb);
}

void Dexdump::AppendCodecConstString(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAAStringBBBB("const-string", ref, sb, dex_file);
}

void Dexdump::AppendCodecConstStringJumbo(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAAStringBBBBBBBB("const-string/jumbo", ref, sb, dex_file);
}

void Dexdump::AppendCodecConstClass(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAATypeBBBB("const-class", ref, sb, dex_file);
}

void Dexdump::AppendCodecMonitorEnter(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("monitor-enter", ref, sb);
}

void Dexdump::AppendCodecMonitorExit(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("monitor-exit", ref, sb);
}

void Dexdump::AppendCodecCheckCast(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAATypeBBBB("check-cast", ref, sb, dex_file);
}

void Dexdump::AppendCodecInstanceOf(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAvBTypeCCCC("instance-of", ref, sb, dex_file);
}

void Dexdump::AppendCodecArrayLength(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvB("array-length", ref, sb);
}

void Dexdump::AppendCodecNewInstance(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAATypeBBBB("new-instance", ref, sb, dex_file);
}

void Dexdump::AppendCodecNewArray(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    AppendCodecArgsvAvBTypeCCCC("new-array", ref, sb, dex_file);
}

void Dexdump::AppendCodecFilledNewArray(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("filled-new-array");
    sb.append(" ");
    int num = (((code0 & 0xFF00) >> 8) & 0xF0) >> 4;
    sb.append("{");
    for (int i = 0; i < num; i++) {
        sb.append("v");
        if (i < 4) {
            sb.append(std::to_string((code2 & (0xF << (i * 4))) >> (i * 4)));
        } else {
            int vreg = (code0 & 0x0F00) >> 8;
            sb.append(std::to_string(vreg));
        }
        if (i != num - 1)
            sb.append(", ");
    }
    sb.append("}, ");
    dex::TypeIndex type(code1);
    dex::TypeId tid = dex_file.GetTypeId(type);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetTypeDescriptor(tid), &result);
    sb.append(result);
    sb.append(" // type@");
    sb.append(std::to_string(type.Index()));
}

void Dexdump::AppendCodecFilledNewArrayRange(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("filled-new-array/range");
    sb.append(" ");
    int num = (code0 & 0xFF00) >> 8;
    uint16_t vaaaa = code2;
    sb.append("{v");
    sb.append(std::to_string(vaaaa));
    sb.append(" .. v");
    sb.append(std::to_string(vaaaa + num - 1));
    sb.append("}, ");
    dex::TypeIndex type(code1);
    dex::TypeId tid = dex_file.GetTypeId(type);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetTypeDescriptor(tid), &result);
    sb.append(result);
    sb.append(" // type@");
    sb.append(std::to_string(type.Index()));
}

void Dexdump::AppendCodecFillArrayData(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBBBBBB("fill-array-data", ref, sb);
}

void Dexdump::AppendCodecThrow(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAA("throw", ref, sb);
}

void Dexdump::AppendCodecGoto(api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    snprintf(codec, sizeof(codec), "%04x                     ", code0);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("goto");
    sb.append(" ");
    int aa = (code0 & 0xFF00) >> 8;
    if (aa & 0x80) {
        uint8_t value = ~((aa & 0x7F) - 0x1) & 0x7F;
        sb.append(Utils::ToHex(ref.Ptr() - (value * 2)));
        sb.append(" //-");
        sb.append(std::to_string(value));
    } else {
        uint8_t value = aa & 0x7F;
        sb.append(Utils::ToHex(ref.Ptr() + (value * 2)));
        sb.append(" //+");
        sb.append(std::to_string(value));
    }
}

void Dexdump::AppendCodecGoto16(api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("goto/16");
    sb.append(" ");
    uint16_t aaaa = code1;
    if (aaaa & 0x8000) {
        uint16_t value = ~((aaaa & 0x7FFF) - 0x1) & 0x7FFF;
        sb.append(Utils::ToHex(ref.Ptr() - (value * 2)));
        sb.append(" //-");
        sb.append(std::to_string(value));
    } else {
        uint8_t value = aaaa & 0x7FFF;
        sb.append(Utils::ToHex(ref.Ptr() + (value * 2)));
        sb.append(" //+");
        sb.append(std::to_string(value));
    }
}

void Dexdump::AppendCodecGoto32(api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("goto/32");
    sb.append(" ");
    uint32_t aaaaaaaa = ((uint32_t)code2 & 0xFFFF) << 16 | code1;
    if (aaaaaaaa & 0x80000000) {
        uint32_t value = ~((aaaaaaaa & 0x7FFFFFFF) - 0x1) & 0x7FFFFFFF;
        sb.append(Utils::ToHex(ref.Ptr() - (value * 2)));
        sb.append(" //-");
        sb.append(std::to_string(value));
    } else {
        uint32_t value = aaaaaaaa & 0x7FFFFFFF;
        sb.append(Utils::ToHex(ref.Ptr() + (value * 2)));
        sb.append(" //+");
        sb.append(std::to_string(value));
    }
}

void Dexdump::AppendCodecPackedSwitch(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBBBBBB("packed-switch", ref, sb);
}

void Dexdump::AppendCodecSparseSwitch(api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAABBBBBBBB("sparse-switch", ref, sb);
}

static const char* kComKinds[] = {
    "cmpl-float",
    "cmpg-float",
    "cmpl-double",
    "cmpg-double",
    "cmp-long",
};

void Dexdump::AppendCodecCmpKind(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBvCC(kComKinds[op-DEXOP::CMPL_FLOAT], ref, sb);
}

static const char* kIfTest[] = {
    "if-eq",
    "if-ne",
    "if-lt",
    "if-ge",
    "if-gt",
    "if-le",
};

void Dexdump::AppendCodecIfTest(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(kIfTest[op-DEXOP::IF_EQ]);
    sb.append(" ");
    uint8_t reg = (code0 & 0xFF00) >> 8;
    uint8_t va = reg & 0x0F;
    uint8_t vb = (reg & 0xF0) >> 4;
    sb.append("v");
    sb.append(std::to_string(va));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vb));
    sb.append(", ");
    uint16_t cccc = code1;
    if (cccc & 0x8000) {
        uint16_t value = ~((cccc & 0x7FFF) - 0x1) & 0x7FFF;
        sb.append(Utils::ToHex((ref.Ptr() - (value * 2))));
        sb.append(" //-");
        sb.append(std::to_string(value));
    } else {
        uint16_t value = cccc & 0x7FFF;
        sb.append(Utils::ToHex((ref.Ptr() + (value * 2))));
        sb.append(" //+");
        sb.append(std::to_string(value));
    }
}

static const char* kIfTestz[] = {
    "if-eqz",
    "if-nez",
    "if-ltz",
    "if-gez",
    "if-gtz",
    "if-lez",
};

void Dexdump::AppendCodecIfTestz(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(kIfTestz[op-DEXOP::IF_EQZ]);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    uint16_t bbbb = code1;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    if (bbbb & 0x8000) {
        uint16_t value = ~((bbbb & 0x7FFF) - 0x1) & 0x7FFF;
        sb.append(Utils::ToHex((ref.Ptr() - (value * 2))));
        sb.append(" //-");
        sb.append(std::to_string(value));
    } else {
        uint16_t value = bbbb & 0x7FFF;
        sb.append(Utils::ToHex((ref.Ptr() + (value * 2))));
        sb.append(" //+");
        sb.append(std::to_string(value));
    }
}

static const char* kArrayOp[] = {
    "aget",
    "aget-wide",
    "aget-object",
    "aget-boolean",
    "aget-byte",
    "aget-char",
    "aget-short",
    "aput",
    "aput-wide",
    "aput-object",
    "aput-boolean",
    "aput-byte",
    "aput-char",
    "aput-short",
};

void Dexdump::AppendCodecArrayOp(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBvCC(kArrayOp[op-DEXOP::AGET], ref, sb);
}

static const char* kIInstanceOp[] {
    "iget",
    "iget-wide",
    "iget-object",
    "iget-boolean",
    "iget-byte",
    "iget-char",
    "iget-short",
    "iput",
    "iput-wide",
    "iput-object",
    "iput-boolean",
    "iput-byte",
    "iput-char",
    "iput-short",
};

void Dexdump::AppendCodecIInstanceOp(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(kIInstanceOp[op-DEXOP::IGET]);
    sb.append(" ");
    uint8_t reg = (code0 & 0xFF00) >> 8;
    uint8_t va = reg & 0x0F;
    uint8_t vb = (reg & 0xF0) >> 4;
    sb.append("v");
    sb.append(std::to_string(va));
    sb.append(", ");
    sb.append("v");
    sb.append(std::to_string(vb));
    sb.append(", ");
    uint16_t idx = code1;
    dex::FieldId fid = dex_file.GetFieldId(idx);
    sb.append(dex_file.GetFieldDeclaringClassDescriptor(fid));
    sb.append(".");
    sb.append(dex_file.GetFieldName(fid));
    sb.append(":");
    sb.append(dex_file.GetFieldTypeDescriptor(fid));
    sb.append(" // field@");
    sb.append(std::to_string(idx));
}

static const char* kSStaticOp[] {
    "sget",
    "sget-wide",
    "sget-object",
    "sget-boolean",
    "sget-byte",
    "sget-char",
    "sget-short",
    "sput",
    "sput-wide",
    "sput-object",
    "sput-boolean",
    "sput-byte",
    "sput-char",
    "sput-short",
};

void Dexdump::AppendCodecSStaicOp(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(kSStaticOp[op-DEXOP::SGET]);
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint16_t idx = code1;
    dex::FieldId fid = dex_file.GetFieldId(idx);
    sb.append(dex_file.GetFieldDeclaringClassDescriptor(fid));
    sb.append(".");
    sb.append(dex_file.GetFieldName(fid));
    sb.append(":");
    sb.append(dex_file.GetFieldTypeDescriptor(fid));
    sb.append(" // field@");
    sb.append(std::to_string(idx));
}

static const char* kInvokeKind[] = {
    "invoke-virtual",
    "invoke-super",
    "invoke-direct",
    "invoke-static",
    "invoke-interface",
};

void Dexdump::AppendCodecInvokeKind(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(kInvokeKind[op-DEXOP::INVOKE_VIRTUAL]);
    sb.append(" ");
    int num = (((code0 & 0xFF00) >> 8) & 0xF0) >> 4;
    sb.append("{");
    for (int i = 0; i < num; i++) {
        sb.append("v");
        if (i < 4) {
            sb.append(std::to_string((code2 & (0xF << (i * 4))) >> (i * 4)));
        } else {
            int vreg = (code0 & 0x0F00) >> 8;
            sb.append(std::to_string(vreg));
        }
        if (i != num - 1)
            sb.append(", ");
    }
    sb.append("}, ");
    uint32_t idx = code1;
    dex::MethodId mid = dex_file.GetMethodId(idx);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetMethodReturnTypeDescriptor(mid), &result);
    sb.append(result);
    result.clear();
    sb.append(" ");
    AppendPrettyDescriptor(dex_file.GetMethodDeclaringClassDescriptor(mid), &result);
    sb.append(result);
    sb.append(".");
    sb.append(dex_file.GetMethodName(mid));
    sb.append(dex_file.PrettyMethodParameters(mid));
    sb.append(" // method@");
    sb.append(std::to_string(idx));
}

static const char* kInvokeKindRange[] = {
    "invoke-virtual/range",
    "invoke-super/range",
    "invoke-direct/range",
    "invoke-static/range",
    "invoke-interface/range",
};

void Dexdump::AppendCodecInvokeKindRange(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append(kInvokeKindRange[op-DEXOP::INVOKE_VIRTUAL_RANGE]);
    sb.append(" ");
    int num = (code0 & 0xFF00) >> 8;
    uint16_t vaaaa = code2;
    sb.append("{v");
    sb.append(std::to_string(vaaaa));
    sb.append(" .. v");
    sb.append(std::to_string(vaaaa + num - 1));
    sb.append("}, ");
    uint32_t idx = code1;
    dex::MethodId mid = dex_file.GetMethodId(idx);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetMethodReturnTypeDescriptor(mid), &result);
    sb.append(result);
    result.clear();
    sb.append(" ");
    AppendPrettyDescriptor(dex_file.GetMethodDeclaringClassDescriptor(mid), &result);
    sb.append(result);
    sb.append(".");
    sb.append(dex_file.GetMethodName(mid));
    sb.append(dex_file.PrettyMethodParameters(mid));
    sb.append(" // method@");
    sb.append(std::to_string(idx));
}

static const char* kUnOp[] {
    "neg-int",
    "not-int",
    "neg-long",
    "not-long",
    "neg-float",
    "neg-double",
    "int-to-long",
    "int-to-float",
    "int-to-double",
    "long-to-int",
    "long-to-float",
    "long-to-double",
    "float-to-int",
    "float-to-long",
    "float-to-double",
    "double-to-int",
    "double-to-long",
    "double-to-float",
    "int-to-byte",
    "int-to-char",
    "int-to-short"
};

void Dexdump::AppendCodecUnOp(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvB(kUnOp[op-DEXOP::NEG_INT], ref, sb);
}

static const char* kBinOp[] = {
    "add-int",
    "sub-int",
    "mul-int",
    "div-int",
    "rem-int",
    "and-int",
    "or-int",
    "xor-int",
    "shl-int",
    "shr-int",
    "ushr-int",
    "add-long",
    "sub-long",
    "mul-long",
    "div-long",
    "rem-long",
    "and-long",
    "or-long",
    "xor-long",
    "shl-long",
    "shr-long",
    "ushr-long",
    "add-float",
    "sub-float",
    "mul-float",
    "div-float",
    "rem-float",
    "add-double",
    "sub-double",
    "mul-double",
    "div-double",
    "rem-double",
};

void Dexdump::AppendCodecBinOp(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBvCC(kBinOp[op-DEXOP::ADD_INT], ref, sb);
}

static const char* kBinOp2Addr[] = {
    "add-int/2addr",
    "sub-int/2addr",
    "mul-int/2addr",
    "div-int/2addr",
    "rem-int/2addr",
    "and-int/2addr",
    "or-int/2addr",
    "xor-int/2addr",
    "shl-int/2addr",
    "shr-int/2addr",
    "ushr-int/2addr",
    "add-long/2addr",
    "sub-long/2addr",
    "mul-long/2addr",
    "div-long/2addr",
    "rem-long/2addr",
    "and-long/2addr",
    "or-long/2addr",
    "xor-long/2addr",
    "shl-long/2addr",
    "shr-long/2addr",
    "ushr-long/2addr",
    "add-float/2addr",
    "sub-float/2addr",
    "mul-float/2addr",
    "div-float/2addr",
    "rem-float/2addr",
    "add-double/2addr",
    "sub-double/2addr",
    "mul-double/2addr",
    "div-double/2addr",
    "rem-double/2addr",
};

void Dexdump::AppendCodecBinOp2Addr(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvB(kBinOp2Addr[op-DEXOP::ADD_INT_2ADDR], ref, sb);
}

static const char* kBinOpLit16[] = {
    "add-int/lit16",
    "rsub-int/lit16",
    "mul-int/lit16",
    "div-int/lit16",
    "rem-int/lit16",
    "and-int/lit16",
    "or-int/lit16",
    "xor-int/lit16",
};

void Dexdump::AppendCodecBinOpLit16(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAvBCCCC(kBinOpLit16[op-DEXOP::ADD_INT_LIT16], ref, sb);
}

static const char* kBinOpLit8[] = {
    "add-int/lit8",
    "rsub-int/lit8",
    "mul-int/lit8",
    "div-int/lit8",
    "rem-int/lit8",
    "and-int/lit8",
    "or-int/lit8",
    "xor-int/lit8",
    "shl-int/lit8",
    "shr-int/lit8",
    "ushr-int/lit8",
};

void Dexdump::AppendCodecBinOpLit8(uint8_t op, api::MemoryRef& ref, std::string& sb) {
    AppendCodecArgsvAAvBBCC(kBinOpLit8[op-DEXOP::ADD_INT_LIT8], ref, sb);
}

void Dexdump::AppendCodecInvokePolymorphic(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    uint16_t code3 = ref.value16Of(8);
    snprintf(codec, sizeof(codec), "%04x %04x %04x %04x      ", code0, code1, code2, code3);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("invoke-polymorphic");
    sb.append(" ");
    int num = (((code0 & 0xFF00) >> 8) & 0xF0) >> 4;
    sb.append("{");
    for (int i = 0; i < num; i++) {
        sb.append("v");
        if (i < 4) {
            sb.append(std::to_string((code2 & (0xF << (i * 4))) >> (i * 4)));
        } else {
            int vreg = (code0 & 0x0F00) >> 8;
            sb.append(std::to_string(vreg));
        }
        if (i != num - 1)
            sb.append(", ");
    }
    sb.append("}, ");
    uint32_t method_idx = code1;
    dex::MethodId mid = dex_file.GetMethodId(method_idx);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetMethodReturnTypeDescriptor(mid), &result);
    sb.append(result);
    result.clear();
    sb.append(" ");
    AppendPrettyDescriptor(dex_file.GetMethodDeclaringClassDescriptor(mid), &result);
    sb.append(result);
    sb.append(".");
    sb.append(dex_file.GetMethodName(mid));
    sb.append(dex_file.PrettyMethodParameters(mid));
    sb.append(", ");
    dex::ProtoIndex proto_idx(code3);
    dex::ProtoId pid = dex_file.GetProtoId(proto_idx);
    dex::TypeIndex return_type_idx(pid.return_type_idx());
    dex::TypeId return_type_id = dex_file.GetTypeId(return_type_idx);
    sb.append(dex_file.GetMethodParametersDescriptor(pid));
    sb.append(dex_file.GetTypeDescriptor(return_type_id));
    sb.append(" // method@");
    sb.append(std::to_string(method_idx));
    sb.append(", ");
    sb.append(" // proto@");
    sb.append(std::to_string(proto_idx.Index()));
}

void Dexdump::AppendCodecInvokePolymorphicRange(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    uint16_t code3 = ref.value16Of(8);
    snprintf(codec, sizeof(codec), "%04x %04x %04x %04x      ", code0, code1, code2, code3);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("invoke-polymorphic/range");
    sb.append(" ");
    int num = (code0 & 0xFF00) >> 8;
    uint16_t vaaaa = code2;
    sb.append("{v");
    sb.append(std::to_string(vaaaa));
    sb.append(" .. v");
    sb.append(std::to_string(vaaaa + num - 1));
    sb.append("}, ");
    uint32_t method_idx = code1;
    dex::MethodId mid = dex_file.GetMethodId(method_idx);
    std::string result;
    AppendPrettyDescriptor(dex_file.GetMethodReturnTypeDescriptor(mid), &result);
    sb.append(result);
    result.clear();
    sb.append(" ");
    AppendPrettyDescriptor(dex_file.GetMethodDeclaringClassDescriptor(mid), &result);
    sb.append(result);
    sb.append(".");
    sb.append(dex_file.GetMethodName(mid));
    sb.append(dex_file.PrettyMethodParameters(mid));
    sb.append(", ");
    dex::ProtoIndex proto_idx(code3);
    dex::ProtoId pid = dex_file.GetProtoId(proto_idx);
    dex::TypeIndex return_type_idx(pid.return_type_idx());
    dex::TypeId return_type_id = dex_file.GetTypeId(return_type_idx);
    sb.append(dex_file.GetMethodParametersDescriptor(pid));
    sb.append(dex_file.GetTypeDescriptor(return_type_id));
    sb.append(" // method@");
    sb.append(std::to_string(method_idx));
    sb.append(", ");
    sb.append(" // proto@");
    sb.append(std::to_string(proto_idx.Index()));
}

void Dexdump::AppendCodecInvokeCustom(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("invoke-custom");
    sb.append(" ");
    int num = (((code0 & 0xFF00) >> 8) & 0xF0) >> 4;
    sb.append("{");
    for (int i = 0; i < num; i++) {
        sb.append("v");
        if (i < 4) {
            sb.append(std::to_string((code2 & (0xF << (i * 4))) >> (i * 4)));
        } else {
            int vreg = (code0 & 0x0F00) >> 8;
            sb.append(std::to_string(vreg));
        }
        if (i != num - 1)
            sb.append(", ");
    }
    sb.append("}, ");
    uint16_t bbbb = code1;
    sb.append(" // call_site@");
    sb.append(std::to_string(bbbb));
}

void Dexdump::AppendCodecInvokeCustomRange(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    uint16_t code2 = ref.value16Of(4);
    snprintf(codec, sizeof(codec), "%04x %04x %04x           ", code0, code1, code2);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("invoke-custom/range");
    sb.append(" ");
    int num = (code0 & 0xFF00) >> 8;
    uint16_t vaaaa = code2;
    sb.append("{v");
    sb.append(std::to_string(vaaaa));
    sb.append(" .. v");
    sb.append(std::to_string(vaaaa + num - 1));
    sb.append("}, ");
    uint16_t bbbb = code1;
    sb.append(" // call_site@");
    sb.append(std::to_string(bbbb));
}

void Dexdump::AppendCodecConstMethodHandle(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("const-method-handle");
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint16_t bbbb = code1;
    sb.append(" // method_handle@");
    sb.append(std::to_string(bbbb));
}

void Dexdump::AppendCodecConstMethodType(api::MemoryRef& ref, std::string& sb, DexFile& dex_file) {
    char codec[128];
    uint16_t code0 = ref.value16Of();
    uint16_t code1 = ref.value16Of(2);
    snprintf(codec, sizeof(codec), "%04x %04x                ", code0, code1);
    sb.append(Logger::LightYellow());
    sb.append(codec);
    sb.append(Logger::End());
    sb.append("| ");
    sb.append(Logger::LightGreen());
    sb.append("const-method-type");
    sb.append(" ");
    uint8_t vaa = (code0 & 0xFF00) >> 8;
    sb.append("v");
    sb.append(std::to_string(vaa));
    sb.append(", ");
    uint16_t bbbb = code1;
    dex::ProtoIndex proto_idx(bbbb);
    dex::ProtoId pid = dex_file.GetProtoId(proto_idx);
    dex::TypeIndex return_type_idx(pid.return_type_idx());
    dex::TypeId return_type_id = dex_file.GetTypeId(return_type_idx);
    sb.append(dex_file.GetMethodParametersDescriptor(pid));
    sb.append(dex_file.GetTypeDescriptor(return_type_id));
    sb.append(" // proto@");
    sb.append(std::to_string(bbbb));
}

std::string Dexdump::PrettyDexInst(api::MemoryRef& ref, DexFile& dex_file) {
    std::string sb;
    char codec[128];
    uint8_t op = GetDexOp(ref);

    sb.append(Logger::LightCyan());
    sb.append(Utils::ToHex(ref.Ptr()));
    sb.append(Logger::End());
    sb.append(": ");

    if (!IsVaildDexOp(op)) {
        snprintf(codec, sizeof(codec), "%04x                     ", ref.value16Of());
        sb.append(Logger::LightYellow());
        sb.append(codec);
        sb.append(Logger::End());
        sb.append("| ");
        sb.append(Logger::LightGreen());
        sb.append("<unknown>");
        sb.append(Logger::End());
        return sb;
    }

    switch (op) {
        case DEXOP::NOP: AppendCodecNop(ref, sb); break;

        case DEXOP::MOVE: AppendCodecMove(ref, sb); break;
        case DEXOP::MOVE_FROM16: AppendCodecMoveFrom16(ref, sb); break;
        case DEXOP::MOVE_16: AppendCodecMove16(ref, sb); break;
        case DEXOP::MOVE_WIDE: AppendCodecMoveWide(ref, sb); break;
        case DEXOP::MOVE_WIDE_FROM16: AppendCodecMoveWideFrom16(ref, sb); break;
        case DEXOP::MOVE_WIDE_16: AppendCodecMoveWide16(ref, sb); break;
        case DEXOP::MOVE_OBJECT: AppendCodecMoveObject(ref, sb); break;
        case DEXOP::MOVE_OBJECT_FROM16: AppendCodecMoveObjectFrom16(ref, sb); break;
        case DEXOP::MOVE_OBJECT_16: AppendCodecMoveObject16(ref, sb); break;
        case DEXOP::MOVE_RESULT: AppendCodecMoveResult(ref, sb); break;
        case DEXOP::MOVE_RESULT_WIDE: AppendCodecMoveResultWide(ref, sb); break;
        case DEXOP::MOVE_RESULT_OBJECT: AppendCodecMoveResultObject(ref, sb); break;
        case DEXOP::MOVE_EXCEPTION: AppendCodecMoveException(ref, sb); break;

        case DEXOP::RETURN_VOID: AppendCodecReturnVoid(ref, sb); break;
        case DEXOP::RETURN: AppendCodecReturn(ref, sb); break;
        case DEXOP::RETURN_WIDE: AppendCodecReturnWide(ref, sb); break;
        case DEXOP::RETURN_OBJECT: AppendCodecReturnObject(ref, sb); break;

        case DEXOP::CONST_4: AppendCodecConst4(ref, sb); break;
        case DEXOP::CONST_16: AppendCodecConst16(ref, sb); break;
        case DEXOP::CONST: AppendCodecConst(ref, sb); break;
        case DEXOP::CONST_HIGH16: AppendCodecConstHigh16(ref, sb); break;
        case DEXOP::CONST_WIDE_16: AppendCodecConstWide16(ref, sb); break;
        case DEXOP::CONST_WIDE_32: AppendCodecConstWide32(ref, sb); break;
        case DEXOP::CONST_WIDE: AppendCodecConstWide(ref, sb); break;
        case DEXOP::CONST_WIDE_HIGH16: AppendCodecConstWideHigh16(ref, sb); break;
        case DEXOP::CONST_STRING: AppendCodecConstString(ref, sb, dex_file); break;
        case DEXOP::CONST_STRING_JUMBO: AppendCodecConstStringJumbo(ref, sb, dex_file); break;
        case DEXOP::CONST_CLASS: AppendCodecConstClass(ref, sb, dex_file); break;

        case DEXOP::MONITOR_ENTER: AppendCodecMonitorEnter(ref, sb); break;
        case DEXOP::MONITOR_EXIT: AppendCodecMonitorExit(ref, sb); break;

        case DEXOP::CHECK_CAST: AppendCodecCheckCast(ref, sb, dex_file); break;
        case DEXOP::INSTANCE_OF: AppendCodecInstanceOf(ref, sb, dex_file); break;
        case DEXOP::ARRAY_LENGTH: AppendCodecArrayLength(ref, sb); break;
        case DEXOP::NEW_INSTANCE: AppendCodecNewInstance(ref, sb, dex_file); break;
        case DEXOP::NEW_ARRAY: AppendCodecNewArray(ref, sb, dex_file); break;
        case DEXOP::FILLED_NEW_ARRAY: AppendCodecFilledNewArray(ref, sb, dex_file); break;
        case DEXOP::FILLED_NEW_ARRAY_RANGE: AppendCodecFilledNewArrayRange(ref, sb, dex_file); break;
        case DEXOP::FILL_ARRAY_DATA: AppendCodecFillArrayData(ref, sb); break;

        case DEXOP::THROW: AppendCodecThrow(ref, sb); break;
        case DEXOP::GOTO: AppendCodecGoto(ref, sb); break;
        case DEXOP::GOTO_16: AppendCodecGoto16(ref, sb); break;
        case DEXOP::GOTO_32: AppendCodecGoto32(ref, sb); break;

        case DEXOP::PACKED_SWITCH: AppendCodecPackedSwitch(ref, sb); break;
        case DEXOP::SPARSE_SWITCH: AppendCodecSparseSwitch(ref, sb); break;

        case DEXOP::CMPL_FLOAT:
        case DEXOP::CMPG_FLOAT:
        case DEXOP::CMPL_DOUBLE:
        case DEXOP::CMPG_DOUBLE:
        case DEXOP::CMP_LONG: AppendCodecCmpKind(op, ref, sb); break;

        case DEXOP::IF_EQ:
        case DEXOP::IF_NE:
        case DEXOP::IF_LT:
        case DEXOP::IF_GE:
        case DEXOP::IF_GT:
        case DEXOP::IF_LE: AppendCodecIfTest(op, ref, sb); break;

        case DEXOP::IF_EQZ:
        case DEXOP::IF_NEZ:
        case DEXOP::IF_LTZ:
        case DEXOP::IF_GEZ:
        case DEXOP::IF_GTZ:
        case DEXOP::IF_LEZ: AppendCodecIfTestz(op, ref, sb); break;

        case DEXOP::AGET:
        case DEXOP::AGET_WIDE:
        case DEXOP::AGET_OBJECT:
        case DEXOP::AGET_BOOLEAN:
        case DEXOP::AGET_BYTE:
        case DEXOP::AGET_CHAR:
        case DEXOP::AGET_SHORT:
        case DEXOP::APUT:
        case DEXOP::APUT_WIDE:
        case DEXOP::APUT_OBJECT:
        case DEXOP::APUT_BOOLEAN:
        case DEXOP::APUT_BYTE:
        case DEXOP::APUT_CHAR:
        case DEXOP::APUT_SHORT: AppendCodecArrayOp(op, ref, sb); break;

        case DEXOP::IGET:
        case DEXOP::IGET_WIDE:
        case DEXOP::IGET_OBJECT:
        case DEXOP::IGET_BOOLEAN:
        case DEXOP::IGET_BYTE:
        case DEXOP::IGET_CHAR:
        case DEXOP::IGET_SHORT:
        case DEXOP::IPUT:
        case DEXOP::IPUT_WIDE:
        case DEXOP::IPUT_OBJECT:
        case DEXOP::IPUT_BOOLEAN:
        case DEXOP::IPUT_BYTE:
        case DEXOP::IPUT_CHAR:
        case DEXOP::IPUT_SHORT: AppendCodecIInstanceOp(op, ref, sb, dex_file); break;

        case DEXOP::SGET:
        case DEXOP::SGET_WIDE:
        case DEXOP::SGET_OBJECT:
        case DEXOP::SGET_BOOLEAN:
        case DEXOP::SGET_BYTE:
        case DEXOP::SGET_CHAR:
        case DEXOP::SGET_SHORT:
        case DEXOP::SPUT:
        case DEXOP::SPUT_WIDE:
        case DEXOP::SPUT_OBJECT:
        case DEXOP::SPUT_BOOLEAN:
        case DEXOP::SPUT_BYTE:
        case DEXOP::SPUT_CHAR:
        case DEXOP::SPUT_SHORT: AppendCodecSStaicOp(op, ref, sb, dex_file); break;

        case DEXOP::INVOKE_VIRTUAL:
        case DEXOP::INVOKE_SUPER:
        case DEXOP::INVOKE_DIRECT:
        case DEXOP::INVOKE_STATIC:
        case DEXOP::INVOKE_INTERFACE: AppendCodecInvokeKind(op, ref, sb, dex_file); break;

        case DEXOP::INVOKE_VIRTUAL_RANGE:
        case DEXOP::INVOKE_SUPER_RANGE:
        case DEXOP::INVOKE_DIRECT_RANGE:
        case DEXOP::INVOKE_STATIC_RANGE:
        case DEXOP::INVOKE_INTERFACE_RANGE: AppendCodecInvokeKindRange(op, ref, sb, dex_file); break;

        case DEXOP::NEG_INT:
        case DEXOP::NOT_INT:
        case DEXOP::NEG_LONG:
        case DEXOP::NOT_LONG:
        case DEXOP::NEG_FLOAT:
        case DEXOP::NEG_DOUBLE:
        case DEXOP::INT_TO_LONG:
        case DEXOP::INT_TO_FLOAT:
        case DEXOP::INT_TO_DOUBLE:
        case DEXOP::LONG_TO_INT:
        case DEXOP::LONG_TO_FLOAT:
        case DEXOP::LONG_TO_DOUBLE:
        case DEXOP::FLOAT_TO_INT:
        case DEXOP::FLOAT_TO_LONG:
        case DEXOP::FLOAT_TO_DOUBLE:
        case DEXOP::DOUBLE_TO_INT:
        case DEXOP::DOUBLE_TO_LONG:
        case DEXOP::DOUBLE_TO_FLOAT:
        case DEXOP::INT_TO_BYTE:
        case DEXOP::INT_TO_CHAR:
        case DEXOP::INT_TO_SHORT: AppendCodecUnOp(op, ref, sb); break;

        case DEXOP::ADD_INT:
        case DEXOP::SUB_INT:
        case DEXOP::MUL_INT:
        case DEXOP::DIV_INT:
        case DEXOP::REM_INT:
        case DEXOP::AND_INT:
        case DEXOP::OR_INT:
        case DEXOP::XOR_INT:
        case DEXOP::SHL_INT:
        case DEXOP::SHR_INT:
        case DEXOP::USHR_INT:
        case DEXOP::ADD_LONG:
        case DEXOP::SUB_LONG:
        case DEXOP::MUL_LONG:
        case DEXOP::DIV_LONG:
        case DEXOP::REM_LONG:
        case DEXOP::AND_LONG:
        case DEXOP::OR_LONG:
        case DEXOP::XOR_LONG:
        case DEXOP::SHL_LONG:
        case DEXOP::SHR_LONG:
        case DEXOP::USHR_LONG:
        case DEXOP::ADD_FLOAT:
        case DEXOP::SUB_FLOAT:
        case DEXOP::MUL_FLOAT:
        case DEXOP::DIV_FLOAT:
        case DEXOP::REM_FLOAT:
        case DEXOP::ADD_DOUBLE:
        case DEXOP::SUB_DOUBLE:
        case DEXOP::MUL_DOUBLE:
        case DEXOP::DIV_DOUBLE:
        case DEXOP::REM_DOUBLE: AppendCodecBinOp(op, ref, sb); break;

        case DEXOP::ADD_INT_2ADDR:
        case DEXOP::SUB_INT_2ADDR:
        case DEXOP::MUL_INT_2ADDR:
        case DEXOP::DIV_INT_2ADDR:
        case DEXOP::REM_INT_2ADDR:
        case DEXOP::AND_INT_2ADDR:
        case DEXOP::OR_INT_2ADDR:
        case DEXOP::XOR_INT_2ADDR:
        case DEXOP::SHL_INT_2ADDR:
        case DEXOP::SHR_INT_2ADDR:
        case DEXOP::USHR_INT_2ADDR:
        case DEXOP::ADD_LONG_2ADDR:
        case DEXOP::SUB_LONG_2ADDR:
        case DEXOP::MUL_LONG_2ADDR:
        case DEXOP::DIV_LONG_2ADDR:
        case DEXOP::REM_LONG_2ADDR:
        case DEXOP::AND_LONG_2ADDR:
        case DEXOP::OR_LONG_2ADDR:
        case DEXOP::XOR_LONG_2ADDR:
        case DEXOP::SHL_LONG_2ADDR:
        case DEXOP::SHR_LONG_2ADDR:
        case DEXOP::USHR_LONG_2ADDR:
        case DEXOP::ADD_FLOAT_2ADDR:
        case DEXOP::SUB_FLOAT_2ADDR:
        case DEXOP::MUL_FLOAT_2ADDR:
        case DEXOP::DIV_FLOAT_2ADDR:
        case DEXOP::REM_FLOAT_2ADDR:
        case DEXOP::ADD_DOUBLE_2ADDR:
        case DEXOP::SUB_DOUBLE_2ADDR:
        case DEXOP::MUL_DOUBLE_2ADDR:
        case DEXOP::DIV_DOUBLE_2ADDR:
        case DEXOP::REM_DOUBLE_2ADDR: AppendCodecBinOp2Addr(op, ref, sb); break;

        case DEXOP::ADD_INT_LIT16:
        case DEXOP::RSUB_INT_LIT16:
        case DEXOP::MUL_INT_LIT16:
        case DEXOP::DIV_INT_LIT16:
        case DEXOP::REM_INT_LIT16:
        case DEXOP::AND_INT_LIT16:
        case DEXOP::OR_INT_LIT16:
        case DEXOP::XOR_INT_LIT16: AppendCodecBinOpLit16(op, ref, sb); break;

        case DEXOP::ADD_INT_LIT8:
        case DEXOP::RSUB_INT_LIT8:
        case DEXOP::MUL_INT_LIT8:
        case DEXOP::DIV_INT_LIT8:
        case DEXOP::REM_INT_LIT8:
        case DEXOP::AND_INT_LIT8:
        case DEXOP::OR_INT_LIT8:
        case DEXOP::XOR_INT_LIT8:
        case DEXOP::SHL_INT_LIT8:
        case DEXOP::SHR_INT_LIT8:
        case DEXOP::USHR_INT_LIT8: AppendCodecBinOpLit8(op, ref, sb); break;

        case DEXOP::INVOKE_POLYMORPHIC: AppendCodecInvokePolymorphic(ref, sb, dex_file); break;
        case DEXOP::INVOKE_POLYMORPHIC_RANGE: AppendCodecInvokePolymorphicRange(ref, sb, dex_file); break;
        case DEXOP::INVOKE_CUSTOM: AppendCodecInvokeCustom(ref, sb, dex_file); break;
        case DEXOP::INVOKE_CUSTOM_RANGE: AppendCodecInvokeCustomRange(ref, sb, dex_file); break;
        case DEXOP::CONST_METHOD_HANDLE: AppendCodecConstMethodHandle(ref, sb, dex_file); break;
        case DEXOP::CONST_METHOD_TYPE: AppendCodecConstMethodType(ref, sb, dex_file); break;
    }
    sb.append(Logger::End());
    return sb;
}

} // namespace art
