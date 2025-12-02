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

#ifndef ANDROID_DALVIK_VM_BYTECODE_H_
#define ANDROID_DALVIK_VM_BYTECODE_H_

namespace DEXOP {

static constexpr int NOP                         = 0x00;            // nop
static constexpr int MOVE                        = 0x01;            // move vA, vB
static constexpr int MOVE_FROM16                 = 0x02;            // move/from16 vAA, vBBBB
static constexpr int MOVE_16                     = 0x03;            // move/16 vAAAA, vBBBB
static constexpr int MOVE_WIDE                   = 0x04;            // move-wide vA, vB
static constexpr int MOVE_WIDE_FROM16            = 0x05;            // move-wide/from16 vAA, vBBBB
static constexpr int MOVE_WIDE_16                = 0x06;            // move-wide/16 vAAAA, vBBBB
static constexpr int MOVE_OBJECT                 = 0x07;            // move-object vA, vB
static constexpr int MOVE_OBJECT_FROM16          = 0x08;            // move-object/from16 vAA, vBBBB
static constexpr int MOVE_OBJECT_16              = 0x09;            // move-object/16 vAAAA, vBBBB
static constexpr int MOVE_RESULT                 = 0x0A;            // move-result vAA
static constexpr int MOVE_RESULT_WIDE            = 0x0B;            // move-result-wide vAA
static constexpr int MOVE_RESULT_OBJECT          = 0x0C;            // move-result-object vAA
static constexpr int MOVE_EXCEPTION              = 0x0D;            // move-exception vAA
static constexpr int RETURN_VOID                 = 0x0E;            // return-void
static constexpr int RETURN                      = 0x0F;            // return vAA
static constexpr int RETURN_WIDE                 = 0x10;            // return-wide vAA
static constexpr int RETURN_OBJECT               = 0x11;            // return-object vAA
static constexpr int CONST_4                     = 0x12;            // const/4 vA, #+B
static constexpr int CONST_16                    = 0x13;            // const/16 vAA, #+BBBB
static constexpr int CONST                       = 0x14;            // const vAA, #+BBBBBBBB
static constexpr int CONST_HIGH16                = 0x15;            // const/high16 vAA, #+BBBB0000
static constexpr int CONST_WIDE_16               = 0x16;            // const-wide/16 vAA, #+BBBB
static constexpr int CONST_WIDE_32               = 0x17;            // const-wide/32 vAA, #+BBBBBBBB
static constexpr int CONST_WIDE                  = 0x18;            // const-wide vAA, #+BBBBBBBBBBBBBBBB
static constexpr int CONST_WIDE_HIGH16           = 0x19;            // const-wide/high16 vAA, #+BBBB000000000000
static constexpr int CONST_STRING                = 0x1A;            // const-string vAA, string@BBBB
static constexpr int CONST_STRING_JUMBO          = 0x1B;            // const-string/jumbo vAA, string@BBBBBBBB
static constexpr int CONST_CLASS                 = 0x1C;            // const-class vAA, type@BBBB
static constexpr int MONITOR_ENTER               = 0x1D;            // monitor-enter vAA
static constexpr int MONITOR_EXIT                = 0x1E;            // monitor-exit vAA
static constexpr int CHECK_CAST                  = 0x1F;            // check-cast vAA, type@BBBB
static constexpr int INSTANCE_OF                 = 0x20;            // instance-of vA, vB, type@CCCC
static constexpr int ARRAY_LENGTH                = 0x21;            // array-length vA, vB
static constexpr int NEW_INSTANCE                = 0x22;            // new-instance vAA, type@BBBB
static constexpr int NEW_ARRAY                   = 0x23;            // new-array vA, vB, type@CCCC
static constexpr int FILLED_NEW_ARRAY            = 0x24;            // filled-new-array {vC, vD, vE, vF, vG}, type@BBBB
static constexpr int FILLED_NEW_ARRAY_RANGE      = 0x25;            // filled-new-array/range {vCCCC .. vNNNN}, type@BBBB
static constexpr int FILL_ARRAY_DATA             = 0x26;            // fill-array-data vAA, +BBBBBBBB
static constexpr int THROW                       = 0x27;            // throw vAA
static constexpr int GOTO                        = 0x28;            // goto +AA
static constexpr int GOTO_16                     = 0x29;            // goto/16 +AAAA
static constexpr int GOTO_32                     = 0x2A;            // goto/32 +AAAAAAAA
static constexpr int PACKED_SWITCH               = 0x2B;            // packed-switch vAA, +BBBBBBBB
static constexpr int SPARSE_SWITCH               = 0x2C;            // sparse-switch vAA, +BBBBBBBB

// cmp kind vAA, vBB, vCC
static constexpr int CMPL_FLOAT                  = 0x2D;            // cmpl-float
static constexpr int CMPG_FLOAT                  = 0x2E;            // cmpg-float
static constexpr int CMPL_DOUBLE                 = 0x2F;            // cmpl-double
static constexpr int CMPG_DOUBLE                 = 0x30;            // cmpg-double
static constexpr int CMP_LONG                    = 0x31;            // cmp-long

// if-test vA, vB, +CCCC
static constexpr int IF_EQ                       = 0x32;            // if-eq
static constexpr int IF_NE                       = 0x33;            // if-ne
static constexpr int IF_LT                       = 0x34;            // if-lt
static constexpr int IF_GE                       = 0x35;            // if-ge
static constexpr int IF_GT                       = 0x36;            // if-gt
static constexpr int IF_LE                       = 0x37;            // if-le

// if-testz vAA, +BBBB
static constexpr int IF_EQZ                      = 0x38;            // if-eqz
static constexpr int IF_NEZ                      = 0x39;            // if-nez
static constexpr int IF_LTZ                      = 0x3A;            // if-ltz
static constexpr int IF_GEZ                      = 0x3B;            // if-gez
static constexpr int IF_GTZ                      = 0x3C;            // if-gtz
static constexpr int IF_LEZ                      = 0x3D;            // if-lez

// arrayop vAA, vBB, vCC
static constexpr int AGET                        = 0x44;            // aget
static constexpr int AGET_WIDE                   = 0x45;            // aget-wide
static constexpr int AGET_OBJECT                 = 0x46;            // aget-object
static constexpr int AGET_BOOLEAN                = 0x47;            // aget-boolean
static constexpr int AGET_BYTE                   = 0x48;            // aget-byte
static constexpr int AGET_CHAR                   = 0x49;            // aget-char
static constexpr int AGET_SHORT                  = 0x4A;            // aget_short
static constexpr int APUT                        = 0x4B;            // aput
static constexpr int APUT_WIDE                   = 0x4C;            // aput-wide
static constexpr int APUT_OBJECT                 = 0x4D;            // aput-object
static constexpr int APUT_BOOLEAN                = 0x4E;            // aput-boolean
static constexpr int APUT_BYTE                   = 0x4F;            // aput-byte
static constexpr int APUT_CHAR                   = 0x50;            // aput-char
static constexpr int APUT_SHORT                  = 0x51;            // aput-short

// iinstanceop vA, vB, field@CCCC
static constexpr int IGET                        = 0x52;            // iget
static constexpr int IGET_WIDE                   = 0x53;            // iget-wide
static constexpr int IGET_OBJECT                 = 0x54;            // iget-object
static constexpr int IGET_BOOLEAN                = 0x55;            // iget-boolean
static constexpr int IGET_BYTE                   = 0x56;            // iget-byte
static constexpr int IGET_CHAR                   = 0x57;            // iget-char
static constexpr int IGET_SHORT                  = 0x58;            // iget_short
static constexpr int IPUT                        = 0x59;            // iput
static constexpr int IPUT_WIDE                   = 0x5A;            // iput-wide
static constexpr int IPUT_OBJECT                 = 0x5B;            // iput-object
static constexpr int IPUT_BOOLEAN                = 0x5C;            // iput-boolean
static constexpr int IPUT_BYTE                   = 0x5D;            // iput-byte
static constexpr int IPUT_CHAR                   = 0x5E;            // iput-char
static constexpr int IPUT_SHORT                  = 0x5F;            // iput-short

// sstaticop vAA, field@BBBB
static constexpr int SGET                        = 0x60;            // sget
static constexpr int SGET_WIDE                   = 0x61;            // sget-wide
static constexpr int SGET_OBJECT                 = 0x62;            // sget-object
static constexpr int SGET_BOOLEAN                = 0x63;            // sget-boolean
static constexpr int SGET_BYTE                   = 0x64;            // sget-byte
static constexpr int SGET_CHAR                   = 0x65;            // sget-char
static constexpr int SGET_SHORT                  = 0x66;            // sget_short
static constexpr int SPUT                        = 0x67;            // sput
static constexpr int SPUT_WIDE                   = 0x68;            // sput-wide
static constexpr int SPUT_OBJECT                 = 0x69;            // sput-object
static constexpr int SPUT_BOOLEAN                = 0x6A;            // sput-boolean
static constexpr int SPUT_BYTE                   = 0x6B;            // sput-byte
static constexpr int SPUT_CHAR                   = 0x6C;            // sput-char
static constexpr int SPUT_SHORT                  = 0x6D;            // sput-short

// invoke-kind {vC, vD, vE, vF, vG}, meth@BBBB
static constexpr int INVOKE_VIRTUAL              = 0x6E;            // invoke-virtual
static constexpr int INVOKE_SUPER                = 0x6F;            // invoke-super
static constexpr int INVOKE_DIRECT               = 0x70;            // invoke-direct
static constexpr int INVOKE_STATIC               = 0x71;            // invoke-static
static constexpr int INVOKE_INTERFACE            = 0x72;            // invoke-interface

static constexpr int RETURN_VOID_NO_BARRIER      = 0x73;            // return-void-no-barrier

// invoke-kind/range {vCCCC .. vNNNN}, meth@BBBB
static constexpr int INVOKE_VIRTUAL_RANGE        = 0x74;            // invoke-virtual/range
static constexpr int INVOKE_SUPER_RANGE          = 0x75;            // invoke-super/range
static constexpr int INVOKE_DIRECT_RANGE         = 0x76;            // invoke-direct/range
static constexpr int INVOKE_STATIC_RANGE         = 0x77;            // invoke-static/range
static constexpr int INVOKE_INTERFACE_RANGE      = 0x78;            // invoke-interface/range

// unop vA, vB
static constexpr int NEG_INT                     = 0x7B;            // neg-int
static constexpr int NOT_INT                     = 0x7C;            // not-int
static constexpr int NEG_LONG                    = 0x7D;            // neg-long
static constexpr int NOT_LONG                    = 0x7E;            // not-long
static constexpr int NEG_FLOAT                   = 0x7F;            // neg-float
static constexpr int NEG_DOUBLE                  = 0x80;            // neg-double
static constexpr int INT_TO_LONG                 = 0x81;            // int-to-long
static constexpr int INT_TO_FLOAT                = 0x82;            // int-to-float
static constexpr int INT_TO_DOUBLE               = 0x83;            // int-to-double
static constexpr int LONG_TO_INT                 = 0x84;            // long-to-int
static constexpr int LONG_TO_FLOAT               = 0x85;            // long-to-float
static constexpr int LONG_TO_DOUBLE              = 0x86;            // long-to-double
static constexpr int FLOAT_TO_INT                = 0x87;            // float-to-int
static constexpr int FLOAT_TO_LONG               = 0x88;            // float-to-long
static constexpr int FLOAT_TO_DOUBLE             = 0x89;            // float-to-double
static constexpr int DOUBLE_TO_INT               = 0x8A;            // double-to-int
static constexpr int DOUBLE_TO_LONG              = 0x8B;            // double-to-long
static constexpr int DOUBLE_TO_FLOAT             = 0x8C;            // double-to-float
static constexpr int INT_TO_BYTE                 = 0x8D;            // int-to-byte
static constexpr int INT_TO_CHAR                 = 0x8E;            // int-to-char
static constexpr int INT_TO_SHORT                = 0x8F;            // int-to-short

// binop vAA, vBB, vCC
static constexpr int ADD_INT                     = 0x90;            // add-int
static constexpr int SUB_INT                     = 0x91;            // sub-int
static constexpr int MUL_INT                     = 0x92;            // mul-int
static constexpr int DIV_INT                     = 0x93;            // div-int
static constexpr int REM_INT                     = 0x94;            // rem-int
static constexpr int AND_INT                     = 0x95;            // and-int
static constexpr int OR_INT                      = 0x96;            // or-int
static constexpr int XOR_INT                     = 0x97;            // xor-int
static constexpr int SHL_INT                     = 0x98;            // shl-int
static constexpr int SHR_INT                     = 0x99;            // shr-int
static constexpr int USHR_INT                    = 0x9A;            // ushr-int
static constexpr int ADD_LONG                    = 0x9B;            // add-long
static constexpr int SUB_LONG                    = 0x9C;            // sub-long
static constexpr int MUL_LONG                    = 0x9D;            // mul-long
static constexpr int DIV_LONG                    = 0x9E;            // div-long
static constexpr int REM_LONG                    = 0x9F;            // rem-long
static constexpr int AND_LONG                    = 0xA0;            // and-long
static constexpr int OR_LONG                     = 0xA1;            // or-long
static constexpr int XOR_LONG                    = 0xA2;            // xor-long
static constexpr int SHL_LONG                    = 0xA3;            // shl-long
static constexpr int SHR_LONG                    = 0xA4;            // shr-long
static constexpr int USHR_LONG                   = 0xA5;            // ushr-long
static constexpr int ADD_FLOAT                   = 0xA6;            // add-float
static constexpr int SUB_FLOAT                   = 0xA7;            // sub-float
static constexpr int MUL_FLOAT                   = 0xA8;            // mul-float
static constexpr int DIV_FLOAT                   = 0xA9;            // div-float
static constexpr int REM_FLOAT                   = 0xAA;            // rem-float
static constexpr int ADD_DOUBLE                  = 0xAB;            // add-double
static constexpr int SUB_DOUBLE                  = 0xAC;            // sub-double
static constexpr int MUL_DOUBLE                  = 0xAD;            // mul-double
static constexpr int DIV_DOUBLE                  = 0xAE;            // div-double
static constexpr int REM_DOUBLE                  = 0xAF;            // rem-double

//binop/2addr vA, vB
static constexpr int ADD_INT_2ADDR               = 0xB0;            // add-int/2addr
static constexpr int SUB_INT_2ADDR               = 0xB1;            // sub-int/2addr
static constexpr int MUL_INT_2ADDR               = 0xB2;            // mul-int/2addr
static constexpr int DIV_INT_2ADDR               = 0xB3;            // div-int/2addr
static constexpr int REM_INT_2ADDR               = 0xB4;            // rem-int/2addr
static constexpr int AND_INT_2ADDR               = 0xB5;            // and-int/2addr
static constexpr int OR_INT_2ADDR                = 0xB6;            // or-int/2addr
static constexpr int XOR_INT_2ADDR               = 0xB7;            // xor-int/2addr
static constexpr int SHL_INT_2ADDR               = 0xB8;            // shl-int/2addr
static constexpr int SHR_INT_2ADDR               = 0xB9;            // shr-int/2addr
static constexpr int USHR_INT_2ADDR              = 0xBA;            // ushr-int/2addr
static constexpr int ADD_LONG_2ADDR              = 0xBB;            // add-long/2addr
static constexpr int SUB_LONG_2ADDR              = 0xBC;            // sub-long/2addr
static constexpr int MUL_LONG_2ADDR              = 0xBD;            // mul-long/2addr
static constexpr int DIV_LONG_2ADDR              = 0xBE;            // div-long/2addr
static constexpr int REM_LONG_2ADDR              = 0xBF;            // rem-long/2addr
static constexpr int AND_LONG_2ADDR              = 0xC0;            // and-long/2addr
static constexpr int OR_LONG_2ADDR               = 0xC1;            // or-long/2addr
static constexpr int XOR_LONG_2ADDR              = 0xC2;            // xor-long/2addr
static constexpr int SHL_LONG_2ADDR              = 0xC3;            // shl-long/2addr
static constexpr int SHR_LONG_2ADDR              = 0xC4;            // shr-long/2addr
static constexpr int USHR_LONG_2ADDR             = 0xC5;            // ushr-long/2addr
static constexpr int ADD_FLOAT_2ADDR             = 0xC6;            // add-float/2addr
static constexpr int SUB_FLOAT_2ADDR             = 0xC7;            // sub-float/2addr
static constexpr int MUL_FLOAT_2ADDR             = 0xC8;            // mul-float/2addr
static constexpr int DIV_FLOAT_2ADDR             = 0xC9;            // div-float/2addr
static constexpr int REM_FLOAT_2ADDR             = 0xCA;            // rem-float/2addr
static constexpr int ADD_DOUBLE_2ADDR            = 0xCB;            // add-double/2addr
static constexpr int SUB_DOUBLE_2ADDR            = 0xCC;            // sub-double/2addr
static constexpr int MUL_DOUBLE_2ADDR            = 0xCD;            // mul-double/2addr
static constexpr int DIV_DOUBLE_2ADDR            = 0xCE;            // div-double/2addr
static constexpr int REM_DOUBLE_2ADDR            = 0xCF;            // rem-double/2addr

// binop/lit16 vA, vB, #+CCCC
static constexpr int ADD_INT_LIT16               = 0xD0;            // add-int/lit16
static constexpr int RSUB_INT_LIT16              = 0xD1;            // rsub-int (reverse subtract)
static constexpr int MUL_INT_LIT16               = 0xD2;            // mul-int/lit16
static constexpr int DIV_INT_LIT16               = 0xD3;            // div-int/lit16
static constexpr int REM_INT_LIT16               = 0xD4;            // rem-int/lit16
static constexpr int AND_INT_LIT16               = 0xD5;            // and-int/lit16
static constexpr int OR_INT_LIT16                = 0xD6;            // or-int/lit16
static constexpr int XOR_INT_LIT16               = 0xD7;            // xor-int/lit16

// binop/lit8 vAA, vBB, #+CC
static constexpr int ADD_INT_LIT8                = 0xD8;            // add-int/lit8
static constexpr int RSUB_INT_LIT8               = 0xD9;            // rsub-int/lit8
static constexpr int MUL_INT_LIT8                = 0xDA;            // mul-int/lit8
static constexpr int DIV_INT_LIT8                = 0xDB;            // div-int/lit8
static constexpr int REM_INT_LIT8                = 0xDC;            // rem-int/lit8
static constexpr int AND_INT_LIT8                = 0xDD;            // and-int/lit8
static constexpr int OR_INT_LIT8                 = 0xDE;            // or-int/lit8
static constexpr int XOR_INT_LIT8                = 0xDF;            // xor-int/lit8
static constexpr int SHL_INT_LIT8                = 0xE0;            // shl-int/lit8
static constexpr int SHR_INT_LIT8                = 0xE1;            // shr-int/lit8
static constexpr int USHR_INT_LIT8               = 0xE2;            // ushr-int/lit8

// iinstanceop vA, vB, offset@CCCC
static constexpr int IGET_QUICK                  = 0xE3;            // iget-quick
static constexpr int IGET_WIDE_QUICK             = 0xE4;            // iget-wide-quick
static constexpr int IGET_OBJECT_QUICK           = 0xE5;            // iget-object-quick
static constexpr int IPUT_QUICK                  = 0xE6;            // iput-quick
static constexpr int IPUT_WIDE_QUICK             = 0xE7;            // iput-wide-quick
static constexpr int IPUT_OBJECT_QUICK           = 0xE8;            // iput-object-quick

static constexpr int INVOKE_VIRTUAL_QUICK        = 0xE9;            // invoke-virtual-quick {vC, vD, vE, vF, vG}, vtable@BBBB
static constexpr int INVOKE_VIRTUAL_QUICK_RANGE  = 0xEA;            // invoke-virtual-quick/range {vCCCC .. vNNNN}, vtable@BBBB

// iinstanceop vA, vB, thing@CCCC
static constexpr int IPUT_BOOLEAN_QUICK          = 0xEB;            // iput-boolean-quick
static constexpr int IPUT_BYTE_QUICK             = 0xEC;            // iput-byte-quick
static constexpr int IPUT_CHAR_QUICK             = 0xED;            // iput-char-quick
static constexpr int IPUT_SHORT_QUICK            = 0xEE;            // iput-short-quick
static constexpr int IGET_BOOLEAN_QUICK          = 0xEF;            // iget-boolean-quick
static constexpr int IGET_BYTE_QUICK             = 0xF0;            // iget-byte-quick
static constexpr int IGET_CHAR_QUICK             = 0xF1;            // iget-char-quick
static constexpr int IGET_SHORT_QUICK            = 0xF2;            // iget-short-quick

static constexpr int INVOKE_POLYMORPHIC          = 0xFA;            // invoke-polymorphic {vC, vD, vE, vF, vG}, meth@BBBB, proto@HHHH
static constexpr int INVOKE_POLYMORPHIC_RANGE    = 0xFB;            // invoke-polymorphic/range {vCCCC .. vNNNN}, meth@BBBB, proto@HHHH
static constexpr int INVOKE_CUSTOM               = 0xFC;            // invoke-custom {vC, vD, vE, vF, vG}, call_site@BBBB
static constexpr int INVOKE_CUSTOM_RANGE         = 0xFD;            // invoke-custom/range {vCCCC .. vNNNN}, call_site@BBBB
static constexpr int CONST_METHOD_HANDLE         = 0xFE;            // const-method-handle vAA, method_handle@BBBB
static constexpr int CONST_METHOD_TYPE           = 0xFF;            // const-method-type vAA, proto@BBBB

} // DEXOP

#endif  // ANDROID_DALVIK_VM_BYTECODE_H_
