/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "common/exception.h"
#include "command/cmd_reference.h"
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

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"global",       no_argument,       0,  0 },
        {"weak",         no_argument,       0,  1 },
        {"hex",          no_argument,       0, 'x'},
    };

    while ((opt = getopt_long(argc, argv, "x",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 0:
                dump_refs = true;
                flags |= Android::EACH_GLOBAL_REFERENCES;
                break;
            case 1:
                dump_refs = true;
                flags |= Android::EACH_WEAK_GLOBAL_REFERENCES;
                break;
            case 'x':
                format_hex = true;
                break;
        }
    }

    if (!flags) {
        flags |= Android::EACH_GLOBAL_REFERENCES;
        flags |= Android::EACH_WEAK_GLOBAL_REFERENCES;
    }

    auto callback = [&](art::mirror::Object& object, uint64_t idx) -> bool {
        std::string descriptor;
        art::mirror::Class thiz = 0x0;
        if (object.IsClass()) {
            thiz = object;
        } else {
            thiz = object.GetClass();
        }
        descriptor = thiz.PrettyDescriptor();

        LOGI("[%ld] " ANSI_COLOR_LIGHTYELLOW  "0x%lx" ANSI_COLOR_LIGHTCYAN " %s\n" ANSI_COLOR_RESET,
                idx, object.Ptr(), descriptor.c_str());

        return false;
    };

    if (optind < argc) {
        art::Runtime& runtime = art::Runtime::Current();
        art::JavaVMExt& jvm = runtime.GetJavaVM();
        uint64_t uref = Utils::atol(argv[optind]);
        art::IndirectRefKind kind = art::IndirectReferenceTable::DecodeIndirectRefKind(uref);
        if (kind == art::IndirectRefKind::kGlobal) {
            reference = jvm.DecodeGlobal(uref);
            LOGI(ANSI_COLOR_LIGHTYELLOW "[JNI_GLOBAL] 0x%lx\n" ANSI_COLOR_RESET, reference.Ptr());
        } else if (kind == art::IndirectRefKind::kWeakGlobal) {
            reference = jvm.DecodeWeakGlobal(uref);
            LOGI(ANSI_COLOR_LIGHTYELLOW "[JNI_WEAK_GLOBAL] 0x%lx\n" ANSI_COLOR_RESET, reference.Ptr());
        }
        int vargc = 2;
        std::string address = Utils::ToHex(reference.Ptr());
        char* vargv[3] = {
            const_cast<char*>("p"),
            const_cast<char*>(address.c_str()),
            const_cast<char*>(""),};
        if (format_hex) {
            vargc++;
            vargv[2] = const_cast<char*>("--hex");
        }
        CommandManager::Execute(vargv[0], vargc, vargv);
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
    LOGI("    --global     foreach global references table\n");
    LOGI("    --weak       foreach weak global references table\n");
    LOGI("    -x, --hex    basic type hex print\n");
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
