/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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
#include "android.h"
#include "command/android/verify.h"
#include "command/android/cmd_print.h"
#include "command/android/cmd_class.h"
#include "runtime/art_method.h"
#include "dexdump/dexdump.h"
#include "dex/descriptors_names.h"
#include "dalvik_vm_bytecode.h"
#include "java/lang/Object.h"
#include "java/lang/Class.h"
#include <vector>

void JavaVerify::init(int opt, int flag) {
    options = opt;
    flags = flag;
}

void JavaVerify::verify(art::mirror::Object& object) {
    art::mirror::Class clazz = object.GetClass();

    if (options & CHECK_FULL_BAD_OBJECT) {
        if (object.IsClass()) {
            art::mirror::Class thiz = object;
            VerifyClassObject(thiz);
        } else if (clazz.IsArrayClass()) {
            art::mirror::Array thiz = object;
            VerifyArrayObject(thiz);
        } else {
            VerifyInstanceObject(object);
        }
    }

    if (options & CHECK_FULL_CONFLICT_METHOD) {
        if (object.IsClass()) {
            art::mirror::Class thiz = object;
            VerifyConflictMethod(thiz);
        }
    }

    if (options & CHECK_FULL_REUSE_DEX_PC_PTR) {
        if (object.IsClass()) {
            art::mirror::Class thiz = object;
            VerifyReuseDexPcMethod(thiz);
        }
    }

    if ((options & CHECK_FULL_U_EXTENDS_RECORD)
            && (Android::Sdk() == Android::U)) {
        if (object.IsClass()) {
            art::mirror::Class thiz = object;
            VerifyRecordClass(thiz);
        }
    }
}

void JavaVerify::VerifyClassObject(art::mirror::Class& clazz) {
    art::mirror::Class current = clazz;
    std::string format = PrintCommand::FormatSize(clazz.SizeOf());
    PrintCommand::Options options;
    bool need_header = true;

    std::vector<art::ArtField> fields;
    auto callback = [&](art::ArtField& field) -> bool {
        fields.push_back(field);
        return false;
    };

    Android::ForeachStaticField(current, callback);
    current = clazz.GetClass();
    if (current.Ptr() && current.IsValid()) {
        Android::ForeachInstanceField(current, callback);
        current = current.GetSuperClass();

        if (current.Ptr() && current.IsValid())
            Android::ForeachInstanceField(current, callback);
    }
    std::sort(fields.begin(), fields.end(), art::ArtField::Compare);

    for (auto& field : fields) {
        uint64_t size;
        const char* sig = field.GetTypeDescriptor();
        Android::BasicType type = Android::SignatureToBasicTypeAndSize(sig, &size, "B");
        if (type != Android::basic_object)
            continue;

        art::mirror::Object tmp(field.GetObj(clazz), clazz);
        if (tmp.Ptr() && !tmp.IsValid()) {
            if (need_header) {
                LOGE("verify class: " ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, clazz.Ptr());
                LOGI("Class Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());
                need_header = false;
            }
            PrintCommand::PrintField(format.c_str(), current, clazz, field, options);
        }
    }
    if (!need_header) ENTER();
}

void JavaVerify::VerifyArrayObject(art::mirror::Array& array) {
    art::mirror::Class clazz = array.GetClass();
    uint32_t length = array.GetLength();
    bool need_header = true;

    if (array.IsObjectArray()) {
        for (int32_t i = 0; i < length; ++i) {
            api::MemoryRef ref(array.GetRawData(sizeof(uint32_t), i), array);
            art::mirror::Object object(*reinterpret_cast<uint32_t *>(ref.Real()), array);
            if (object.Ptr() && !object.IsValid()) {
                if (need_header) {
                    LOGE("verify array: " ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, array.Ptr());
                    LOGI("Array Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());
                    need_header = false;
                }
                LOGI("    [%d] " ANSI_COLOR_LIGHTMAGENTA "0x%" PRIx64 "\n" ANSI_COLOR_RESET, i, object.Ptr());
            }
        }
    }
    if (!need_header) ENTER();
}

void JavaVerify::VerifyInstanceObject(art::mirror::Object& object) {
    art::mirror::Class clazz = object.GetClass();
    art::mirror::Class super = clazz;
    std::string format = PrintCommand::FormatSize(object.SizeOf());
    PrintCommand::Options options;
    bool need_header = true;
    do {
        std::vector<art::ArtField> fields;
        auto callback = [&](art::ArtField& field) -> bool {
            fields.push_back(field);
            return false;
        };
        Android::ForeachInstanceField(super, callback);
        std::sort(fields.begin(), fields.end(), art::ArtField::Compare);

        for (auto& field : fields) {
            uint64_t size;
            const char* sig = field.GetTypeDescriptor();
            Android::BasicType type = Android::SignatureToBasicTypeAndSize(sig, &size, "B");
            if (type != Android::basic_object)
                continue;

            art::mirror::Object tmp(field.GetObj(object), object);
            if (tmp.Ptr() && !tmp.IsValid()) {
                if (need_header) {
                    LOGE("verify instance: " ANSI_COLOR_LIGHTYELLOW "[0x%" PRIx64 "]\n" ANSI_COLOR_RESET, object.Ptr());
                    LOGI("Object Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, clazz.PrettyDescriptor().c_str());
                    need_header = false;
                }
                PrintCommand::PrintField(format.c_str(), super, object, field, options);
            }
        }

        super = super.GetSuperClass();
    } while (super.Ptr());
    if (!need_header) ENTER();
}

void JavaVerify::VerifyConflictMethod(art::mirror::Class& clazz) {
    ClassCommand::Options options = {
        .show_flag = ClassCommand::SHOW_IMPL,
    };
    bool need_header = true;
    auto print_method = [&](art::ArtMethod& method) -> bool {
        if (!method.IsDefaultConflicting())
            return false;

        if (need_header) {
            LOGE("verify class conflict method\n");
            ClassCommand::PrintPrettyClassContent(clazz, options);
            need_header = false;
        }
        LOGI("[0x%" PRIx64 "] " ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_RESET "%s\n",
                method.Ptr(), art::PrettyMethodAccessFlags(method.access_flags()).c_str(), method.ColorPrettyMethod().c_str());
        return false;
    };
    Android::ForeachArtMethods(clazz, print_method);
}

void JavaVerify::VerifyReuseDexPcMethod(art::mirror::Class& clazz) {
    auto has_code_method = [&](art::ArtMethod& method) -> bool {
        if (!method.HasCodeItem())
            return false;

        if (method.IsDefaultConflicting())
            return false;

        art::dex::CodeItem item = method.GetCodeItem();
        if (!item.Ptr())
            return false;

        methods.insert(std::pair<uint64_t, uint64_t>(method.Ptr(), item.Ptr() + item.code_offset_));
        return false;
    };
    Android::ForeachArtMethods(clazz, has_code_method);
}

uint64_t JavaVerify::FindSuperMethodToCall(art::ArtMethod& method, uint16_t dex_method_idx) {
    uint64_t result = 0;
    art::mirror::Class thiz = method.GetDeclaringClass();
    art::mirror::Class super = thiz.GetSuperClass();

    art::DexFile& dex_file = thiz.GetDexFile();
    art::dex::MethodId mid = dex_file.GetMethodId(dex_method_idx);

    std::string name;
    name.append(dex_file.GetMethodName(mid));
    name.append(dex_file.PrettyMethodParameters(mid));

    std::string descriptor;
    art::AppendPrettyDescriptor(dex_file.GetMethodDeclaringClassDescriptor(mid), &descriptor);
    java::lang::Class resolve = java::lang::Class::forName(descriptor.c_str());
    art::mirror::Class resolve_class = resolve.thiz();

    auto has_super_method = [&](art::ArtMethod& super_method) -> bool {
        std::string super_method_pretty;
        super_method_pretty.append(super_method.GetName());
        super_method_pretty.append(super_method.PrettyParameters());
        if (super_method_pretty == name) {
            result = super_method.Ptr();
            return true;
        }
        return false;
    };

    if (resolve_class.Ptr() && resolve_class.IsInterface()) {
        art::mirror::IfTable& iftable = thiz.GetIfTable();
        int32_t ifcount = (iftable.Ptr() && iftable.IsValid()) ? iftable.Count() : 0;
        if (ifcount) {
            for (int i = 0; i < ifcount; ++i) {
                art::mirror::Class interface = iftable.GetInterface(i);
                Android::ForeachArtMethods(interface, has_super_method);
                if (result) break;
            }
        }
    }

    if (!result) {
        do {
            Android::ForeachArtMethods(super, has_super_method);
            if (result) break;
            super = super.GetSuperClass();
        } while (super.Ptr());
    }
    return result;
}

void JavaVerify::verifyMethods() {
    if (!(options & CHECK_FULL_REUSE_DEX_PC_PTR))
        return;

    std::unordered_map<uint64_t, uint64_t> dex_pc_ptrs;
    std::unordered_map<uint64_t, std::vector<uint64_t>> reuse_dexpc_method;
    for (auto& value : methods) {
        uint64_t method = value.first;
        uint64_t dex_pc_ptr = value.second;

        if (!dex_pc_ptrs.count(dex_pc_ptr)) {
            dex_pc_ptrs.insert(std::pair<uint64_t, uint64_t>(dex_pc_ptr, method));
        } else {
            auto it = reuse_dexpc_method.find(dex_pc_ptr);
            if (it == reuse_dexpc_method.end()) {
                std::vector<uint64_t> tmp_methods;
                tmp_methods.push_back(dex_pc_ptrs[dex_pc_ptr]);
                tmp_methods.push_back(method);
                reuse_dexpc_method.insert(std::pair<uint64_t, std::vector<uint64_t>>(dex_pc_ptr, tmp_methods));
            } else {
                it->second.push_back(method);
            }
        }
    }

    for (auto& entry : reuse_dexpc_method) {
        art::ArtMethod tmp = entry.second[0];
        art::mirror::Class thiz = tmp.GetDeclaringClass();
        art::DexFile& dex_file = thiz.GetDexFile();

        art::dex::CodeItem item = tmp.GetCodeItem();
        api::MemoryRef coderef = item.Ptr() + item.code_offset_;
        api::MemoryRef endref = coderef.Ptr() + (item.insns_count_ << 1);

        bool maybe_warn = false;
        std::vector<uint16_t> super_methods;
        while (coderef < endref) {
            uint8_t op = art::Dexdump::GetDexOp(coderef);
            if (op == DEXOP::INVOKE_SUPER || op == DEXOP::INVOKE_SUPER_RANGE) {
                std::string super;
                uint16_t code1 = coderef.value16Of(2);
                // art::dex::MethodId mid = dex_file.GetMethodId(code1);
                // super.append(dex_file.GetMethodName(mid));
                // super.append(dex_file.PrettyMethodParameters(mid));
                super_methods.push_back(code1);
                maybe_warn = true;
            }
            coderef.MovePtr(art::Dexdump::GetDexInstSize(coderef));
        }

        if (maybe_warn) {
            bool other_extends = false;
            for (auto& value : entry.second) {
                if (tmp.Ptr() == value)
                    continue;

                art::ArtMethod method = value;
                art::mirror::Class clazz = method.GetDeclaringClass();
                art::mirror::Class clazz_super = clazz.GetSuperClass();
                art::mirror::Class thiz_super = thiz.GetSuperClass();

                if (thiz_super.Ptr() != clazz_super.Ptr()) {
                    other_extends = true;
                    break;
                }
            }

            if (other_extends) {
                bool other_super = false;
                for (auto& idx : super_methods) {
                    uint64_t tmp_super = FindSuperMethodToCall(tmp, idx);
                    for (auto& value : entry.second) {
                        if (tmp.Ptr() == value)
                            continue;

                        art::ArtMethod method = value;
                        uint64_t method_super = FindSuperMethodToCall(method, idx);

                        if (tmp_super != method_super) {
                            other_super = true;
                            break;
                        }
                    }
                }

                if (other_super) {
                    uint64_t dex_pc_ptr = entry.first;
                    LOGE("verify class reuse dex_pc_ptr method\n");
                    for (auto& value : entry.second) {
                        art::ArtMethod method = value;
                        LOGI("[0x%" PRIx64 "] " ANSI_COLOR_LIGHTGREEN "%s" ANSI_COLOR_RESET "%s\n",
                                method.Ptr(), art::PrettyMethodAccessFlags(method.access_flags()).c_str(), method.ColorPrettyMethod().c_str());
                        for (auto& idx : super_methods) {
                            art::ArtMethod resolve_method = FindSuperMethodToCall(method, idx);
                            if (resolve_method.Ptr())
                                LOGD("    |--> invoke [0x%" PRIx64 "](%s)\n", resolve_method.Ptr(), resolve_method.ColorPrettyMethod().c_str());
                        }
                    }
                    ENTER();
                }
            }
        }
    }
}

void JavaVerify::VerifyRecordClass(art::mirror::Class& clazz) {
    java::lang::Object thiz = clazz;
    if (clazz.IsRecordClass() || thiz.mirror_instanceof("java.lang.Record")) {
        std::string classname = clazz.PrettyDescriptor();
        if (classname != "java.lang.Record") {
            LOGE("verify class [0x%" PRIx64 "] extends java.lang.Record\n", clazz.Ptr());
            LOGI("Class Name: " ANSI_COLOR_LIGHTMAGENTA "%s\n" ANSI_COLOR_RESET, classname.c_str());
            ENTER();
        }
    }
}
