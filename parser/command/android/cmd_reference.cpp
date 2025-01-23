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

#include "logger/log.h"
#include "common/exception.h"
#include "command/android/cmd_print.h"
#include "command/android/cmd_reference.h"
#include "command/command_manager.h"
#include "runtime/runtime.h"
#include "runtime/jni/java_vm_ext.h"
#include "api/core.h"
#include "base/utils.h"
#include "android.h"
#include <unistd.h>
#include <getopt.h>

int ReferenceCommand::main(int argc, char* const argv[]) {
    if (!CoreApi::IsReady() || !Android::IsSdkReady())
        return 0;

    bool dump_refs = true;
    int flags = 0;
    bool format_hex = false;
    art::mirror::Object reference = 0x0;
    int tid = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"local",        no_argument,       0,  0 },
        {"global",       no_argument,       0,  1 },
        {"weak",         no_argument,       0,  2 },
        {"hex",          no_argument,       0, 'x'},
        {"thread", required_argument,       0, 't'},
    };

    while ((opt = getopt_long(argc, argv, "xt:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                dump_refs = true;
                flags |= Android::EACH_LOCAL_REFERENCES;
                break;
            case 1:
                dump_refs = true;
                flags |= Android::EACH_GLOBAL_REFERENCES;
                break;
            case 2:
                dump_refs = true;
                flags |= Android::EACH_WEAK_GLOBAL_REFERENCES;
                break;
            case 'x':
                format_hex = true;
                break;
            case 't':
                tid = std::atoi(optarg);
                flags |= (tid << Android::EACH_LOCAL_REFERENCES_BY_TID_SHIFT);
                break;
        }
    }

    if (!(flags & ((1 << Android::EACH_LOCAL_REFERENCES_BY_TID_SHIFT) - 1))) {
        flags |= Android::EACH_LOCAL_REFERENCES;
        flags |= Android::EACH_GLOBAL_REFERENCES;
        flags |= Android::EACH_WEAK_GLOBAL_REFERENCES;
    }

    auto callback = [&](art::mirror::Object& object, int type, uint64_t iref) -> bool {
        std::string descriptor;
        art::mirror::Class thiz = 0x0;
        if (object.IsClass()) {
            thiz = object;
        } else {
            thiz = object.GetClass();
        }
        descriptor = thiz.PrettyDescriptor();

        art::IndirectRefKind kind = static_cast<art::IndirectRefKind>(type & ((1 << Android::EACH_LOCAL_REFERENCES_BY_TID_SHIFT) - 1));
        if (kind == art::IndirectRefKind::kLocal)
            LOGI("[%d]", type >> Android::EACH_LOCAL_REFERENCES_BY_TID_SHIFT);
        LOGI("[%s][0x%04" PRIx64 "] " ANSI_COLOR_LIGHTYELLOW  "0x%" PRIx64 "" ANSI_COLOR_LIGHTCYAN " %s\n" ANSI_COLOR_RESET,
                art::IndirectReferenceTable::GetDescriptor(kind).c_str(), iref, object.Ptr(), descriptor.c_str());

        return false;
    };

    if (optind < argc) {
        art::Runtime& runtime = art::Runtime::Current();
        art::JavaVMExt& jvm = runtime.GetJavaVM();
        uint64_t uref = Utils::atol(argv[optind]);
        reference = jvm.Decode(uref);
        art::IndirectRefKind kind = art::IndirectReferenceTable::DecodeIndirectRefKind(uref);
        if (kind && kind != art::IndirectRefKind::kLocal) {
            reference = jvm.Decode(uref);
            LOGI("[%s][0x%04" PRIx64 "] " ANSI_COLOR_LIGHTYELLOW  "0x%" PRIx64 "" ANSI_COLOR_RESET "\n",
                    art::IndirectReferenceTable::GetDescriptor(kind).c_str(), uref, reference.Ptr());
            PrintCommand::OnlyDumpObject(reference, format_hex);
        } else if (kind == art::IndirectRefKind::kLocal) {
            art::ThreadList& thread_list = runtime.GetThreadList();
            for (const auto& thread : thread_list.GetList()) {
                art::JNIEnvExt& jni_env = thread->GetJNIEnv();
                reference = jni_env.Decode(uref);
                if (!reference.Ptr())
                    continue;

                if (tid && tid != thread->GetTid())
                    continue;

                LOGI("[%d]\n", thread->GetTid());
                LOGI("[%s][0x%04" PRIx64 "] " ANSI_COLOR_LIGHTYELLOW  "0x%" PRIx64 "" ANSI_COLOR_RESET "\n",
                        art::IndirectReferenceTable::GetDescriptor(kind).c_str(), uref, reference.Ptr());
                PrintCommand::OnlyDumpObject(reference, format_hex);
            }
        }
    } else {
        try {
            Android::ForeachReferences(callback, flags);
        } catch(InvalidAddressException e) {
            LOGW("The statistical process was interrupted!\n");
        }
    }
    return 0;
}

void ReferenceCommand::usage() {
    LOGI("Usage: reference|ref [<UREF>] [OPTIONE]\n");
    LOGI("Option:\n");
    LOGI("        --local [-t|..]  foreach thread local references table\n");
    LOGI("    -t, --thread <TID>   filter local references by tid\n");
    LOGI("        --global         foreach global references table\n");
    LOGI("        --weak           foreach weak global references table\n");
    LOGI("    -x, --hex            basic type hex print\n");
    ENTER();
    LOGI("core-parser> ref 2206 -x\n");
    LOGI("[JNI_GLOBAL] 0x71c9c4c0\n");
    LOGI("Size: 0x40\n");
    LOGI("Padding: 0x4\n");
    LOGI("Object Name: java.nio.DirectByteBuffer\n");
    LOGI("    [0x38] final java.nio.DirectByteBuffer$MemoryRef memoryRef = 0x71fadd78\n");
    LOGI("    [0x34] final sun.misc.Cleaner cleaner = 0x0\n");
    LOGI("  // extends java.nio.MappedByteBuffer\n");
    LOGI("    [0x30] private final java.io.FileDescriptor fd = 0x0\n");
    LOGI("  // extends java.nio.ByteBuffer\n");
    LOGI("    [0x2e] boolean nativeByteOrder = false\n");
    LOGI("    [0x2d] boolean isReadOnly = true\n");
    LOGI("    [0x2c] boolean bigEndian = true\n");
    LOGI("    [0x28] final int offset = 0x0\n");
    LOGI("    [0x24] final byte[] hb = 0x0\n");
    LOGI("  // extends java.nio.Buffer\n");
    LOGI("    [0x20] int position = 0x0\n");
    LOGI("    [0x1c] private int mark = 0xffffffff\n");
    LOGI("    [0x18] private int limit = 0x6930\n");
    LOGI("    [0x14] private int capacity = 0x6930\n");
    LOGI("    [0x10] final int _elementSizeShift = 0x0\n");
    LOGI("    [0x08] long address = 0x7dfc54e42000\n");
    LOGI("  // extends java.lang.Object\n");
    LOGI("    [0x04] private transient int shadow$_monitor_ = 0x20000000\n");
    LOGI("    [0x00] private transient java.lang.Class shadow$_klass_ = 0x6f917db0\n");
}
