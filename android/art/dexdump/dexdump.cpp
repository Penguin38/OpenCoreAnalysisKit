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
#include <stdio.h>

namespace art {

uint32_t Dexdump::GetDexInstSize(api::MemoryRef& ref) {
    uint8_t op = GetDexOp(ref);
    if (!IsVaildDexOp(op))
        return 0x2;

    switch (op) {
        case DEXOP::NOP: return 0x2;
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
    char codec[32];
    snprintf(codec, sizeof(codec), "%04x                     | ", ref.value16Of());
    sb.append(codec);
    sb.append(op);
}

void Dexdump::AppendCodecNop(api::MemoryRef& ref, std::string& sb) {
    AppendCodecNoArgs("nop", ref, sb);
}

void Dexdump::AppendCodecReturnVoid(api::MemoryRef& ref, std::string& sb) {
    AppendCodecNoArgs("return-void", ref, sb);
}

std::string Dexdump::PrettyDexInst(api::MemoryRef& ref, DexFile& dex_file) {
    std::string sb;
    char codec[32];
    uint8_t op = GetDexOp(ref);

    sb.append(Utils::ToHex(ref.Ptr()));
    sb.append(": ");

    if (!IsVaildDexOp(op)) {
        snprintf(codec, sizeof(codec), "%04x                     | ", ref.value16Of());
        sb.append(codec);
        sb.append("<unknown>");
        return sb;
    }

    switch (op) {
        case DEXOP::NOP: AppendCodecNop(ref, sb); break;
        case DEXOP::MOVE:
        case DEXOP::MOVE_FROM16:
        case DEXOP::MOVE_16:
        case DEXOP::MOVE_WIDE:
        case DEXOP::MOVE_WIDE_FROM16:
        case DEXOP::MOVE_WIDE_16:
        case DEXOP::MOVE_OBJECT:
        case DEXOP::MOVE_OBJECT_FROM16:
        case DEXOP::MOVE_OBJECT_16:

        case DEXOP::MOVE_RESULT:
        case DEXOP::MOVE_RESULT_WIDE:
        case DEXOP::MOVE_RESULT_OBJECT:
        case DEXOP::MOVE_EXCEPTION:

        case DEXOP::RETURN_VOID: AppendCodecReturnVoid(ref, sb); break;
        case DEXOP::RETURN:
        case DEXOP::RETURN_WIDE:
        case DEXOP::RETURN_OBJECT:

        case DEXOP::CONST_4:
        case DEXOP::CONST_16:
        case DEXOP::CONST:
        case DEXOP::CONST_HIGH16:
        case DEXOP::CONST_WIDE_16:
        case DEXOP::CONST_WIDE_32:
        case DEXOP::CONST_WIDE:
        case DEXOP::CONST_WIDE_HIGH16:
        case DEXOP::CONST_STRING:
        case DEXOP::CONST_STRING_JUMBO:
        case DEXOP::CONST_CLASS:

        case DEXOP::MONITOR_ENTER:
        case DEXOP::MONITOR_EXIT:

        case DEXOP::CHECK_CAST:
        case DEXOP::INSTANCE_OF:
        case DEXOP::ARRAY_LENGTH:
        case DEXOP::NEW_INSTANCE:
        case DEXOP::NEW_ARRAY:
        case DEXOP::FILLED_NEW_ARRAY:
        case DEXOP::FILLED_NEW_ARRAY_RANGE:
        case DEXOP::FILL_ARRAY_DATA:

        case DEXOP::THROW:
        case DEXOP::GOTO:
        case DEXOP::GOTO_16:
        case DEXOP::GOTO_32:

        case DEXOP::PACKED_SWITCH:
        case DEXOP::SPARSE_SWITCH:

        case DEXOP::CMPL_FLOAT:
        case DEXOP::CMPG_FLOAT:
        case DEXOP::CMPL_DOUBLE:
        case DEXOP::CMPG_DOUBLE:
        case DEXOP::CMP_LONG:

        case DEXOP::IF_EQ:
        case DEXOP::IF_NE:
        case DEXOP::IF_LT:
        case DEXOP::IF_GE:
        case DEXOP::IF_GT:
        case DEXOP::IF_LE:

        case DEXOP::IF_EQZ:
        case DEXOP::IF_NEZ:
        case DEXOP::IF_LTZ:
        case DEXOP::IF_GEZ:
        case DEXOP::IF_GTZ:
        case DEXOP::IF_LEZ:

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
        case DEXOP::APUT_SHORT:

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
        case DEXOP::IPUT_SHORT:

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
        case DEXOP::SPUT_SHORT:

        case DEXOP::INVOKE_VIRTUAL:
        case DEXOP::INVOKE_SUPER:
        case DEXOP::INVOKE_DIRECT:
        case DEXOP::INVOKE_STATIC:
        case DEXOP::INVOKE_INTERFACE:

        case DEXOP::INVOKE_VIRTUAL_RANGE:
        case DEXOP::INVOKE_SUPER_RANGE:
        case DEXOP::INVOKE_DIRECT_RANGE:
        case DEXOP::INVOKE_STATIC_RANGE:
        case DEXOP::INVOKE_INTERFACE_RANGE:

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
        case DEXOP::INT_TO_SHORT:

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
        case DEXOP::REM_DOUBLE:

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
        case DEXOP::REM_DOUBLE_2ADDR:

        case DEXOP::ADD_INT_LIT16:
        case DEXOP::RSUB_INT_LIT16:
        case DEXOP::MUL_INT_LIT16:
        case DEXOP::DIV_INT_LIT16:
        case DEXOP::REM_INT_LIT16:
        case DEXOP::AND_INT_LIT16:
        case DEXOP::OR_INT_LIT16:
        case DEXOP::XOR_INT_LIT16:

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
        case DEXOP::USHR_INT_LIT8:

        case DEXOP::INVOKE_POLYMORPHIC:
        case DEXOP::INVOKE_POLYMORPHIC_RANGE:
        case DEXOP::INVOKE_CUSTOM:
        case DEXOP::INVOKE_CUSTOM_RANGE:
        case DEXOP::CONST_METHOD_HANDLE:
        case DEXOP::CONST_METHOD_TYPE:
            break;
    }

    return sb;
}

} // namespace art
