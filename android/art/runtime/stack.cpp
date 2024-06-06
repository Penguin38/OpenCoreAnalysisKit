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
#include "android.h"
#include "common/exception.h"
#include "runtime/runtime.h"
#include "runtime/stack.h"
#include "runtime/nterp_helpers.h"
#include "runtime/managed_stack.h"
#include "runtime/art_method.h"
#include "runtime/base/callee_save_type.h"
#include "runtime/entrypoints/quick/callee_save_frame.h"

namespace art {

QuickMethodFrameInfo StackVisitor::GetCurrentQuickFrameInfo() {
    if (cur_oat_quick_method_header_.Ptr()) {
        if(Android::Sdk() > Android::Q) {
            if (cur_oat_quick_method_header_.IsOptimized()) {
                return cur_oat_quick_method_header_.GetFrameInfo();
            } else {
                return NterpFrameInfo(cur_quick_frame_);
            }
        } else {
            return cur_oat_quick_method_header_.GetFrameInfo();
        }
    }

    ArtMethod method = GetMethod();
    Runtime& runtime = Runtime::Current();

    if (method.IsAbstract()) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
    }

    if (method.IsRuntimeMethod()) {
        return runtime.GetRuntimeMethodFrameInfo(method);
    }

    if (method.IsProxyMethod()) {
        return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
    }

    return RuntimeCalleeSaveFrame::GetMethodFrameInfo(CalleeSaveType::kSaveRefsAndArgs);
}

ArtMethod StackVisitor::GetMethod() {
    if (cur_shadow_frame_.Ptr()) {
        return cur_shadow_frame_.GetMethod();
    } else if (cur_quick_frame_.Ptr()) {
        return cur_quick_frame_.GetMethod();
    }
    return 0x0;
}

bool StackVisitor::VisitFrame() {
    ArtMethod method = GetMethod();
    if (method.IsRuntimeMethod())
        return true;

    std::unique_ptr<JavaFrame> frame =
            std::make_unique<JavaFrame>(method,
                                        cur_quick_frame_,
                                        cur_shadow_frame_);
    java_frames_.push_back(std::move(frame));
    return true;
}

void StackVisitor::WalkStack() {
    for (ManagedStack current_fragment = GetThread()->GetTlsPtr().managed_stack();
            current_fragment.Ptr(); current_fragment = current_fragment.link()) {
        try {
            LOGD("  ManagedStack* 0x%lx\n", current_fragment.Ptr());
            cur_shadow_frame_ = current_fragment.GetTopShadowFrame();
            cur_quick_frame_ = current_fragment.GetTopQuickFrame();
            cur_quick_frame_pc_ = 0;

            if (cur_quick_frame_.Ptr()) {
                ArtMethod method = cur_quick_frame_.valueOf();
                bool header_retrieved = false;
                if (method.IsNative()) {
                    if (current_fragment.GetTopQuickFrameGenericJniTag()) {
                        cur_oat_quick_method_header_ = 0x0;
                    } else if (current_fragment.GetTopQuickFrameJitJniTag()) {
                        // JIT TODO
                    } else {
                        uint64_t existing_entry_point = method.GetEntryPointFromQuickCompiledCode();
                        Runtime& runtime = Runtime::Current();
                        ClassLinker& class_linker = runtime.GetClassLinker();
                        if (!class_linker.IsQuickGenericJniStub(existing_entry_point) &&
                            !class_linker.IsQuickResolutionStub(existing_entry_point)) {
                            cur_oat_quick_method_header_ = OatQuickMethodHeader::FromEntryPoint(existing_entry_point);
                        } else {
                            // very few TODO
                        }
                    }
                    header_retrieved = true;
                }

                while (method.Ptr()) {
                    if (!header_retrieved) {
                        cur_oat_quick_method_header_ = method.GetOatQuickMethodHeader(cur_quick_frame_pc_);
                    }
                    header_retrieved = false;

                    if ((walk_kind_ == StackWalkKind::kIncludeInlinedFrames)) {
                        // do nothing
                    }

                    bool should_continue = VisitFrame();
                    if (!should_continue) {
                        return;
                    }

                    QuickMethodFrameInfo frame_info = GetCurrentQuickFrameInfo();
                    uint32_t frame_size = frame_info.FrameSizeInBytes();
                    api::MemoryRef return_pc_addr = frame_info.GetReturnPcAddr(cur_quick_frame_.Ptr());

                    cur_quick_frame_pc_ = return_pc_addr.valueOf();
                    QuickFrame next_frame(cur_quick_frame_.Ptr() + frame_size, cur_quick_frame_);

                    method = next_frame.GetMethod();
                }
                cur_oat_quick_method_header_ = 0x0;
            } else if (cur_shadow_frame_.Ptr()) {
                do {
                    bool should_continue = VisitFrame();
                    if (!should_continue) {
                        return;
                    }
                    cur_shadow_frame_ = cur_shadow_frame_.link();
                } while(cur_shadow_frame_.Ptr());
            }
        } catch(InvalidAddressException e) {
            LOGI("  ManagedStack* 0x%lx maybe invalid.\n", current_fragment.Ptr());
        }
    }
}

} // namespace art
