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

#ifndef ANDROID_ART_DEXDUMP_DEXDUMP_H_
#define ANDROID_ART_DEXDUMP_DEXDUMP_H_

#include "dex/dex_file.h"

namespace art {

class Dexdump {
public:
    inline static uint8_t GetDexOp(api::MemoryRef& ref) { return ref.value8Of(); }
    inline static bool IsVaildDexOp(uint8_t op) {
        if ((op >= 0x3E && op <= 0x43)
                || op == 0x73
                || (op >= 0x79 && op <= 0x7A)
                || (op >= 0xE3 && op <= 0xF9)) {
            return false;
        }
        return true;
    }
    static uint32_t GetDexInstSize(api::MemoryRef& ref);
    static std::string PrettyDexInst(api::MemoryRef& ref, DexFile& dex_file);
    static void AppendCodecNoArgs(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAvB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAAvBBBB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAAAAvBBBB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAA(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAABBBB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAABBBBBBBB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAABBBB0000(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAABBBBBBBBBBBBBBBB(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAABBBB000000000000(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAAStringBBBB(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecArgsvAAStringBBBBBBBB(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecArgsvAATypeBBBB(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecArgsvAvBTypeCCCC(const char* op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecArgsAA(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsAAAA(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsAAAAAAAA(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAAvBBvCC(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAvBCCCC(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArgsvAAvBBCC(const char* op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecNop(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMove(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveFrom16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMove16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveWide(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveWideFrom16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveWide16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveObject(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveObjectFrom16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveObject16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveResult(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveResultWide(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveResultObject(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMoveException(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecReturnVoid(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecReturn(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecReturnWide(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecReturnObject(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConst4(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConst16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConst(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConstHigh16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConstWide16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConstWide32(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConstWide(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConstWideHigh16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecConstString(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecConstStringJumbo(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecConstClass(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecMonitorEnter(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecMonitorExit(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecCheckCast(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecInstanceOf(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecArrayLength(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecNewInstance(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecNewArray(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecFilledNewArray(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecFilledNewArrayRange(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecFillArrayData(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecThrow(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecGoto(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecGoto16(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecGoto32(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecPackedSwitch(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecSparseSwitch(api::MemoryRef& ref, std::string& sb);
    static void AppendCodecCmpKind(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecIfTest(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecIfTestz(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecArrayOp(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecIInstanceOp(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecSStaicOp(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecInvokeKind(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecInvokeKindRange(uint8_t op, api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecUnOp(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecBinOp(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecBinOp2Addr(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecBinOpLit16(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecBinOpLit8(uint8_t op, api::MemoryRef& ref, std::string& sb);
    static void AppendCodecInvokePolymorphic(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecInvokePolymorphicRange(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecInvokeCustom(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecInvokeCustomRange(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecConstMethodHandle(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
    static void AppendCodecConstMethodType(api::MemoryRef& ref, std::string& sb, DexFile& dex_file);
};

} // namespace art

#endif  // ANDROID_ART_DEXDUMP_DEXDUMP_H_
